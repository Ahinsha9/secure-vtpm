import { useState, useRef, useEffect } from 'react'
import './App.css'

/* Virtual TPM Simulator - Educational Software Simulation.
   MOCK MODE: everything below runs in the browser. It mirrors the C++/driver
   logic described in the project README but is NOT connected to /dev/vtpm_secure. */

const enc = new TextEncoder()
const sha = async s => [...new Uint8Array(await crypto.subtle.digest('SHA-256', enc.encode(s)))].map(b => b.toString(16).padStart(2, '0')).join('')
const Z = '0'.repeat(64)
const wait = ms => new Promise(r => setTimeout(r, ms))
const pc = p => (p === 'VALID' ? 'g' : p === 'INVALID' ? 'r' : 'a')
const STAGES = ['User request', 'C++ client', 'ioctl()', 'Linux driver', 'Authorization', 'VTPM core', 'Key / PCR manager', 'Security decision']
const CMDS = { STATUS: 1, GENERATE_KEY: 2, LIST_KEYS: 3, READ_PCR: 4, EXTEND_PCR: 5, SIGN: 6, VERIFY: 7, DELETE_KEY: 8 }

function createVtpm() {
  const v = { keys: [], next: 1001, pcrs: Array(8).fill(Z), trusted: null, sigs: {} }
  const policy = () => (v.trusted === null ? 'NOT SET' : v.trusted === v.pcrs[0] ? 'VALID' : 'INVALID')
  const good = (msg, data) => ({ ok: true, msg, data })
  const bad = (msg, stage = 3) => ({ ok: false, msg, stage })
  const pcrOk = p => Number.isInteger(p) && p >= 0 && p <= 7
  const ops = {
    STATUS: async () => good(`VTPM active, ${v.keys.length} key(s), PCR policy ${policy()}`),
    GENERATE_KEY: async () => { const id = v.next++; v.keys.push(id); return good(`RSA-2048 key ${id} generated. Private key protected, stored encrypted.`) },
    LIST_KEYS: async () => good(`Keys: ${v.keys.join(', ') || 'none'}`),
    READ_PCR: async ({ pcr }) => { const p = Number(pcr); return pcrOk(p) ? good(`PCR${p} = ${v.pcrs[p]}`) : bad(`EINVAL: PCR${pcr} out of range 0-7`) },
    EXTEND_PCR: async ({ pcr, m }) => {
      const p = Number(pcr); if (!pcrOk(p)) return bad(`EINVAL: PCR${pcr} out of range 0-7`)
      const before = v.pcrs[p]; v.pcrs[p] = await sha(before + m)
      return good(`PCR${p} extended: SHA256(old + measurement)`, { p, before, after: v.pcrs[p] })
    },
    ESTABLISH_POLICY: async () => { v.trusted = v.pcrs[0]; return good('Trusted PCR0 recorded. PCR policy established.') },
    CHECK_POLICY: async () => { const p = policy(); return p === 'VALID' ? good('PCR policy VALID: PCR0 matches trusted value') : bad(`PCR policy ${p}`, 7) },
    SIGN: async ({ id, msg }) => {
      const k = Number(id); if (!(k > 0)) return bad('EINVAL: key id must be > 0')
      const p = policy()
      if (p === 'NOT SET') return bad('Denied: no PCR policy established', 7)
      if (p === 'INVALID') return bad('PCR policy mismatch. Protected signing denied.', 7)
      if (!v.keys.includes(k)) return bad(`Key ${k} not found`, 6)
      const sig = await sha(`k${k}|${msg}`); v.sigs[k] = sig
      return good(`PCR policy verified. Signing authorized. Signature ${sig.slice(0, 24)}... (simulated)`)
    },
    VERIFY: async ({ id, msg }) => {
      const k = Number(id); if (!(k > 0)) return bad('EINVAL: key id must be > 0')
      if (!v.sigs[k]) return bad(`No signature stored for key ${k}. Sign first.`, 6)
      return (await sha(`k${k}|${msg}`)) === v.sigs[k] ? good('Signature VALID') : bad('Signature INVALID: message was modified', 6)
    },
    DELETE_KEY: async ({ id }) => {
      const k = Number(id); if (!(k > 0)) return bad('EINVAL: key id must be > 0')
      if (!v.keys.includes(k)) return bad(`Key ${k} not found`, 6)
      v.keys = v.keys.filter(x => x !== k); delete v.sigs[k]; return good(`Key ${k} deleted (encrypted key file removed)`)
    },
  }
  const call = async (op, a = {}) => (a.unauth ? bad('EPERM: caller is not root or in the vtpm group', 4) : ops[op](a))
  return { v, call, policy }
}

function useVtpm() {
  const vt = useRef(null)
  if (!vt.current) vt.current = createVtpm()
  const [, bump] = useState(0)
  const [log, setLog] = useState([{ t: new Date().toLocaleTimeString([], { hour12: false }), m: 'VTPM initialized (simulated)', k: 'info' }])
  const [pipe, setPipe] = useState({ cur: -1, fail: -1 })
  const L = (m, k = 'info') => setLog(l => [...l, { t: new Date().toLocaleTimeString([], { hour12: false }), m, k }].slice(-80))
  const run = async (op, args = {}) => {
    L(`command received: ${op}`)
    const r = await vt.current.call(op, args)
    const end = r.ok ? 7 : r.stage
    for (let i = 0; i <= end; i++) { setPipe({ cur: i, fail: -1 }); await wait(180) }
    setPipe(r.ok ? { cur: 8, fail: -1 } : { cur: -1, fail: end })
    L(`${op}: ${r.msg}`, r.ok ? 'ok' : 'err'); bump(); return r
  }
  const reset = () => { vt.current = createVtpm(); setPipe({ cur: -1, fail: -1 }); L('VTPM re-initialized (simulated)'); bump() }
  return { v: vt.current.v, pol: () => vt.current.policy(), run, reset, log, pipe }
}

const B = ({ c, children }) => <span className={'badge ' + c}>{children}</span>
const Res = ({ r }) => r && <div className={'res ' + (r.ok ? 'g' : 'r')}>{r.ok ? '✓' : '✗'} {r.msg}</div>
const LogBox = ({ entries }) => <div className="log">{entries.map((e, i) => <div key={i} className={e.k}>[{e.t}] {e.m}</div>)}</div>

function Pipeline({ pipe }) {
  const lim = pipe.fail >= 0 ? pipe.fail : pipe.cur
  return <div className="pipe">{STAGES.map((s, i) => {
    const c = i === pipe.fail ? 'bad' : i < lim ? 'ok' : i === pipe.cur ? 'run' : ''
    return <div key={s} className={'stage ' + c}><b>{c === 'bad' ? '✗' : c === 'ok' ? '✓' : '·'}</b>{s}</div>
  })}</div>
}

function Dashboard({ s }) {
  const p = s.pol()
  const sign = p === 'VALID' ? 'allowed' : p === 'INVALID' ? 'blocked' : 'denied (no policy)'
  const cards = [['VTPM status', 'Active', 'g'], ['Driver', 'Connected (simulated)', 'g'], ['PCR policy', p, pc(p)], ['RSA keys', s.v.keys.length, 'b'], ['PCR registers', 8, 'b'], ['Security', p === 'INVALID' ? 'Signing blocked' : 'Protected', p === 'INVALID' ? 'r' : 'g']]
  return <>
    <h2>Dashboard</h2>
    <div className="cards">{cards.map(([a, b, c]) => <div key={a} className={'card ' + c}><small>{a}</small><h3>{b}</h3></div>)}</div>
    <div className="grid2">
      <div className="panel"><h4>Trust chain</h4><div className="chain">
        <div>VTPM core</div>
        <div className={p === 'INVALID' ? 'r' : p === 'VALID' ? 'g' : 'a'}>PCR0 {p === 'VALID' ? 'trusted' : p === 'INVALID' ? 'modified' : 'no baseline yet'}</div>
        <div className={pc(p)}>PCR policy {p.toLowerCase()}</div>
        <div className={p === 'VALID' ? 'g' : 'r'}>Protected signing {sign}</div>
      </div></div>
      <div className="panel"><h4>Recent activity</h4><LogBox entries={s.log.slice(-8)} /></div>
    </div>
  </>
}

const OPS = ['STATUS', 'GENERATE_KEY', 'LIST_KEYS', 'READ_PCR', 'EXTEND_PCR', 'ESTABLISH_POLICY', 'CHECK_POLICY', 'SIGN', 'VERIFY', 'DELETE_KEY']
function Simulation({ s }) {
  const [op, setOp] = useState('SIGN'), [id, setId] = useState(1001), [msg, setMsg] = useState('Secure VTPM Test'), [pcr, setPcr] = useState(0), [unauth, setU] = useState(false), [res, setRes] = useState(null), [busy, setBusy] = useState(false)
  const need = { SIGN: 'id msg', VERIFY: 'id msg', DELETE_KEY: 'id', READ_PCR: 'pcr', EXTEND_PCR: 'pcr msg' }[op] || ''
  const go = async () => { setBusy(true); setRes(await s.run(op, { id, msg, m: msg, pcr, unauth })); setBusy(false) }
  return <>
    <h2>Simulation</h2>
    <div className="grid2">
      <div className="panel">
        <label>Operation</label><select value={op} onChange={e => setOp(e.target.value)}>{OPS.map(o => <option key={o}>{o}</option>)}</select>
        {need.includes('id') && <><label>Key ID</label><input type="number" value={id} onChange={e => setId(e.target.value)} /></>}
        {need.includes('pcr') && <><label>PCR index</label><input type="number" value={pcr} onChange={e => setPcr(e.target.value)} /></>}
        {need.includes('msg') && <><label>Message / measurement</label><input value={msg} onChange={e => setMsg(e.target.value)} /></>}
        <label><input type="checkbox" checked={unauth} onChange={e => setU(e.target.checked)} /> Run as user outside the vtpm group</label>
        <button className="go" disabled={busy} onClick={go}>Execute</button>
        <Res r={res} />
      </div>
      <Pipeline pipe={s.pipe} />
    </div>
  </>
}

function KeyManager({ s }) {
  const [msg, setMsg] = useState('Secure VTPM Test'), [res, setRes] = useState(null)
  const act = async (op, id) => setRes(await s.run(op, { id, msg }))
  return <>
    <h2>Key Manager</h2>
    <div className="panel">
      <button className="go" onClick={() => act('GENERATE_KEY')}>Generate RSA key</button>
      <input value={msg} onChange={e => setMsg(e.target.value)} /> <small>Message for Sign / Verify. Signing needs a PCR policy (PCR Monitor).</small>
    </div>
    <Res r={res} />
    <table><thead><tr>{['Key ID', 'Type', 'Size', 'Status', 'Storage', 'Actions'].map(h => <th key={h}>{h}</th>)}</tr></thead><tbody>
      {s.v.keys.map(id => <tr key={id}><td>{id}</td><td>RSA</td><td>2048</td><td><B c="g">PROTECTED</B></td><td><B c="b">ENCRYPTED</B></td>
        <td>{['SIGN', 'VERIFY', 'DELETE_KEY'].map(o => <button key={o} onClick={() => act(o, id)}>{o.split('_')[0].toLowerCase()}</button>)}</td></tr>)}
      {!s.v.keys.length && <tr><td colSpan="6">No keys yet. Generate one to start.</td></tr>}
    </tbody></table>
    <div className="panel mono">Private key<br />Status: PROTECTED<br />Export: DISABLED<br />Storage: ENCRYPTED</div>
  </>
}

function PCRMonitor({ s }) {
  const [pcr, setPcr] = useState(0), [m, setM] = useState('Unauthorized System Modification'), [ba, setBa] = useState(null)
  const p = s.pol()
  const ext = async () => { const r = await s.run('EXTEND_PCR', { pcr, m }); if (r.ok) setBa(r.data) }
  return <>
    <h2>PCR Monitor</h2>
    <div className="panel">
      <h4>PCR0 policy check</h4>
      <div className="mono hash">Trusted: {s.v.trusted ?? '(not established)'}<br />Current: {s.v.pcrs[0]}</div>
      <p><B c={pc(p)}>{p === 'VALID' ? '✓ Match: policy valid' : p === 'INVALID' ? '✗ Mismatch: policy invalid' : 'No policy set'}</B>{' '}
        {p === 'INVALID' && <B c="r">Protected signing blocked</B>}</p>
      <button className="go" onClick={() => s.run('ESTABLISH_POLICY')}>Establish trusted PCR0 policy</button>
      <button onClick={() => s.run('CHECK_POLICY')}>Check policy</button>
    </div>
    <table><tbody>{s.v.pcrs.map((x, i) => <tr key={i}><td>PCR{i}</td><td className="mono hash">{x}</td></tr>)}</tbody></table>
    <div className="panel">
      <h4>Extend PCR</h4>
      <label>PCR index</label><input type="number" min="0" max="7" value={pcr} onChange={e => setPcr(e.target.value)} />
      <label>Measurement</label><input size="40" value={m} onChange={e => setM(e.target.value)} />
      <button className="go" onClick={ext}>Extend PCR</button>
      {ba && <div className="mono hash">PCR{ba.p} before: {ba.before}<br />measurement, SHA-256<br />PCR{ba.p} after: {ba.after}</div>}
    </div>
  </>
}

function Commands({ s }) {
  const [sel, setSel] = useState(null)
  const go = async c => { setSel(c); await s.run(c, { id: s.v.keys[0] ?? 1001, pcr: 0, m: 'Command Center test', msg: 'Command Center test' }) }
  return <>
    <h2>Command Center</h2>
    <div className="grid2">
      <div className="panel">{Object.entries(CMDS).map(([c, n]) => <button key={c} className={'cmd' + (sel === c ? ' sel' : '')} onClick={() => go(c)}>{c} <small>(command {n})</small></button>)}</div>
      <Pipeline pipe={s.pipe} />
    </div>
    <div className="panel"><LogBox entries={s.log.slice(-6)} /></div>
  </>
}

function Security({ s }) {
  const p = s.pol()
  const sg = p === 'VALID' ? ['g', '✓ Allowed'] : p === 'INVALID' ? ['r', '✗ Blocked'] : ['a', '– Denied until a policy exists']
  const L = [
    ['Linux device permissions', 'g', '✓ Active (simulated)', '/dev/vtpm_secure is mode 0660, group vtpm. Other users get no access.'],
    ['Driver authorization', 'g', '✓ Active (simulated)', 'Only root or vtpm group members pass. Try the "outside the vtpm group" option in Simulation.'],
    ['Encrypted key storage', 'g', '✓ Active (simulated)', 'Keys are stored as AES-256-CBC encrypted PEM. Private keys are never exported.'],
    ['PCR security policy', pc(p), p === 'VALID' ? '✓ Valid' : p === 'INVALID' ? '✗ Invalid' : '– Not set', 'Current PCR0 must equal the trusted PCR0 recorded when the policy was established.'],
    ['Protected signing', sg[0], sg[1], 'Signing needs an active VTPM, a valid policy and an existing key.'],
  ]
  return <><h2>Security</h2>{L.map(([n, c, st, d]) => <div key={n} className="panel"><B c={c}>{st}</B> <b>{n}</b><div style={{ color: 'var(--mut)', marginTop: 6 }}>{d}</div></div>)}</>
}

const ARCH = [
  ['User / UI', 'React frontend (this app)', 'Currently a mock engine. A browser cannot open /dev/vtpm_secure, so an API bridge is still needed.'],
  ['C++ client', 'client/vtpm_cli.cpp, client/VTPMDevice.cpp', 'VTPMDevice: connectDevice(), ping(), getStatus(), authorizeCommand(), isConnected(), disconnectDevice()'],
  ['ioctl()', 'driver/vtpm_ioctl.h', 'VTPM_IOCTL_GET_STATUS, VTPM_IOCTL_PING, VTPM_IOCTL_COMMAND (_IOWR) carrying struct vtpm_command {command_id, parameter, result}'],
  ['Linux driver', 'driver/vtpm_driver.c', 'open(), release(), unlocked ioctl. Authorizes (root or vtpm group), validates (PCR 0-7, key id > 0, unknown commands rejected), uses copy_from_user / copy_to_user. Does not perform RSA.'],
  ['VTPM core', 'src/VTPM.cpp', 'initialize(), isActive(), showStatus(), generateKey(), deleteKey(), signMessage(), establishPCRPolicy(), isPCRPolicyValid()'],
  ['Key manager', 'src/KeyManager.cpp', 'generateKey(), deleteKey(), signMessage(), verifySignature(). RSA-2048, SHA-256, encrypted PEM in data/keys/'],
  ['PCR manager', 'src/PCRManager.cpp', 'initialize(), extendPCR(), getPCRValue(). newPCR = SHA256(oldPCR + measurement)'],
  ['Crypto (OpenSSL)', 'OpenSSL EVP API, userspace', 'EVP_DigestSignInit, EVP_DigestVerifyInit. A separate CryptoManager is planned; check the source before relying on it.'],
]
function Architecture() {
  const [i, setI] = useState(3)
  return <>
    <h2>Architecture</h2>
    <div className="grid2">
      <div className="arch">{ARCH.map((a, k) => <div key={a[0]}>{k > 0 && <i>↓</i>}<button className={k === i ? 'sel' : ''} onClick={() => setI(k)}>{a[0]}</button></div>)}</div>
      <div className="panel"><h4>{ARCH[i][0]}</h4><p className="mono">{ARCH[i][1]}</p><p>{ARCH[i][2]}</p><small style={{ color: 'var(--mut)' }}>Names come from the project README; confirm against the source files.</small></div>
    </div>
  </>
}

const SIGN_ARGS = { id: 1001, msg: 'Secure VTPM Test' }
const STEPS = [
  ['Initialize Virtual TPM', async s => { s.reset(); return { ok: true, msg: 'VTPM active, PCR0-PCR7 initialized' } }],
  ['Generate RSA-2048 key', s => s.run('GENERATE_KEY')],
  ['Establish trusted PCR0', s => s.run('ESTABLISH_POLICY')],
  ['Perform protected signing', s => s.run('SIGN', SIGN_ARGS)],
  ['Simulate unauthorized modification', s => s.run('EXTEND_PCR', { pcr: 0, m: 'Unauthorized System Modification' })],
  ['PCR policy becomes invalid', s => s.run('CHECK_POLICY')],
  ['Attempt protected signing', s => s.run('SIGN', SIGN_ARGS)],
]
function Demo({ s }) {
  const [rs, setRs] = useState([]), [busy, setBusy] = useState(false)
  const next = async () => { setBusy(true); const r = await STEPS[rs.length][1](s); setRs(a => [...a, r]); setBusy(false) }
  const play = async () => { setBusy(true); setRs([]); const out = []; for (const [, f] of STEPS) { out.push(await f(s)); setRs([...out]); await wait(700) } setBusy(false) }
  const done = rs.length === STEPS.length
  return <>
    <h2>Guided Demo</h2>
    <div className="panel">
      <button className="go" disabled={busy} onClick={play}>Play all steps</button>
      <button disabled={busy || done} onClick={next}>Next step</button>
      <button disabled={busy} onClick={() => { setRs([]); s.reset() }}>Reset</button>
    </div>
    <div className="grid2">
      <div>{rs.map((r, i) => <div className="step" key={i}><b>Step {i + 1}/8: {STEPS[i][0]}</b><Res r={r} /></div>)}
        {done && <div className="panel"><h4>Step 8/8: Security demonstration complete</h4>
          {['Trusted state established', 'RSA key protected', 'PCR change detected', 'Unauthorized signing prevented'].map(t => <div key={t} className="res g" style={{ margin: '4px 0' }}>✓ {t}</div>)}</div>}</div>
      <Pipeline pipe={s.pipe} />
    </div>
  </>
}

const TESTS = [
  ['Driver test', async t => (await t.call('STATUS')).ok],
  ['RSA key generation', async t => (await t.call('GENERATE_KEY')).ok],
  ['Key persistence', async t => { await t.call('GENERATE_KEY'); return t.v.keys.includes(1001) }],
  ['Sign / verify', async t => { await t.call('GENERATE_KEY'); await t.call('ESTABLISH_POLICY'); await t.call('SIGN', { id: 1001, msg: 'hi' }); return (await t.call('VERIFY', { id: 1001, msg: 'hi' })).ok && !(await t.call('VERIFY', { id: 1001, msg: 'hI' })).ok }],
  ['PCR extension', async t => { const b = t.v.pcrs[0]; await t.call('EXTEND_PCR', { pcr: 0, m: 'x' }); return t.v.pcrs[0] !== b }],
  ['PCR policy', async t => { await t.call('ESTABLISH_POLICY'); const a = t.policy() === 'VALID'; await t.call('EXTEND_PCR', { pcr: 0, m: 'x' }); return a && t.policy() === 'INVALID' }],
  ['Access control', async t => !(await t.call('STATUS', { unauth: true })).ok],
  ['Invalid PCR', async t => !(await t.call('READ_PCR', { pcr: 10 })).ok],
  ['Invalid key', async t => !(await t.call('SIGN', { id: 0, msg: 'x' })).ok],
  ['Key deletion', async t => { await t.call('GENERATE_KEY'); await t.call('DELETE_KEY', { id: 1001 }); return !t.v.keys.length }],
]
function Tests() {
  const [r, setR] = useState({}), [busy, setBusy] = useState(false)
  const all = async () => { setBusy(true); const out = {}; setR({}); for (const [n, f] of TESTS) { try { out[n] = await f(createVtpm()) } catch { out[n] = false } setR({ ...out }); await wait(150) } setBusy(false) }
  return <>
    <h2>Test Center</h2>
    <div className="panel"><button className="go" disabled={busy} onClick={all}>Run all tests</button> <small>These run against the in-browser simulation, not the C++ test programs.</small></div>
    <table><tbody>{TESTS.map(([n]) => <tr key={n}><td>{n}</td><td>{n in r ? <B c={r[n] ? 'g' : 'r'}>{r[n] ? '✓ PASS' : '✗ FAIL'}</B> : '-'}</td></tr>)}</tbody></table>
  </>
}

const PAGES = [['🏠', 'Dashboard', Dashboard], ['⚙', 'Simulation', Simulation], ['🔑', 'Key Manager', KeyManager], ['🧬', 'PCR Monitor', PCRMonitor], ['⚡', 'Command Center', Commands], ['🛡', 'Security', Security], ['🏗', 'Architecture', Architecture], ['🎓', 'Guided Demo', Demo], ['🧪', 'Test Center', Tests]]

export default function App() {
  const s = useVtpm()
  const [pg, setPg] = useState('Dashboard'), [open, setOpen] = useState(true)
  const logRef = useRef(null)
  useEffect(() => { if (logRef.current) logRef.current.scrollTop = logRef.current.scrollHeight }, [s.log, open])
  const P = PAGES.find(p => p[1] === pg)[2]
  return <div className="app">
    <nav><h1>Virtual TPM<small>Educational software simulation</small></h1>
      {PAGES.map(([ic, n]) => <button key={n} className={n === pg ? 'on' : ''} onClick={() => setPg(n)}>{ic} {n}</button>)}</nav>
    <div className="right">
      <div className="mock">Mock mode: values come from an in-browser simulation, not the real C++ backend or /dev/vtpm_secure. Not a hardware TPM.</div>
      <main><P s={s} /></main>
      <div className="logwrap"><button onClick={() => setOpen(!open)}>{open ? '▾' : '▸'} System log</button>
        {open && <div ref={logRef} style={{ maxHeight: 130, overflow: 'auto' }}><LogBox entries={s.log} /></div>}</div>
    </div>
  </div>
}
