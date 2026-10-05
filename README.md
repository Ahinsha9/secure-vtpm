# 🔐 Secure Hardware Key Enclave Simulator — Virtual TPM

A Linux-based **Virtual Trusted Platform Module (vTPM)** simulator designed to demonstrate secure hardware key-enclave concepts, cryptographic key management, protected device access, Linux kernel-driver interaction, and secure userspace communication.

## 📌 Project Overview

A **Trusted Platform Module (TPM)** is a security component designed to provide protected storage and cryptographic operations for sensitive information such as encryption keys, authentication credentials, and digital signatures.

This project implements an educational **Virtual TPM (vTPM)** environment on Linux.

Instead of requiring a physical TPM chip, this project simulates the key concepts of a hardware security module using:

- A Linux kernel driver
- A virtual TPM device
- A userspace vTPM application
- A client application
- Cryptographic key storage
- A web-based frontend

The project demonstrates how a security request can travel through different layers of the system while controlling access to sensitive cryptographic operations.


# 🎯 Objectives

The main objectives of this project are:

- Implement a software-based Virtual TPM environment.
- Demonstrate secure cryptographic key handling.
- Demonstrate Linux kernel and userspace interaction.
- Create a protected virtual device interface.
- Control access to the virtual TPM using Linux permissions.
- Demonstrate secure key storage.
- Provide a client application for interacting with the vTPM.
- Provide a graphical frontend for project demonstration.
- Provide complete Linux installation and execution instructions.

# ✨ Key Features

### 🔐 Secure Key Management

The project uses a dedicated directory for cryptographic key material:

data/keys/

Private/generated key files are excluded from GitHub using `.gitignore`.

### 🐧 Linux Kernel Driver

The project contains a Linux driver under:

driver/

The driver provides the low-level interface for communicating with the virtual TPM device.

### 💻 Virtual TPM

The core vTPM implementation is contained within:

src/

with the vTPM executable:

vtpm

### 🖥️ Client Application

The client implementation is located under:

client/

The primary client executable is:

vtpm_cli

It can be started using:

./vtpm_cli

### 🌐 Frontend

The project contains a web frontend under:

frontend/

The frontend provides a graphical way to demonstrate and visualize the vTPM system.

# 🏗️ System Architecture

The overall architecture is:

                         ┌──────────────────────┐
                         │        USER          │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │       FRONTEND       │
                         │      /frontend       │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │        CLIENT        │
                         │     ./vtpm_cli       │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │        vTPM          │
                         │       ./vtpm         │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │    LINUX DRIVER      │
                         │       /driver        │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │  /dev/vtpm_secure    │
                         │   Virtual Device     │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │    SECURE KEY        │
                         │      STORAGE         │
                         │     /data/keys       │
                         └──────────────────────┘

# 📂 Project Structure

secure-vtpm/
│
├── client/
│   └── vTPM client application
│
├── data/
│   └── keys/
│       └── Runtime cryptographic key storage
│
├── driver/
│   └── Linux kernel driver
│
├── frontend/
│   └── Web-based frontend
│
├── src/
│   └── vTPM source code
│
├── vtpm
│   └── vTPM executable
│
└── .gitignore

# 🛠️ Technologies Used

## Operating System

- Linux
- Ubuntu recommended

## Programming / System Technologies

- C / C++
- Linux Kernel
- Linux Kernel Modules
- Character Device Interface
- Userspace Applications
- File Permissions

## Security

- Cryptographic Key Management
- Public/Private Key Concepts
- Secure Key Storage
- Access Control
- Protected Device Access

## Frontend

- Node.js
- npm
- JavaScript / TypeScript
- Web-based UI

## Development Tools

- Git
- GitHub
- GCC
- Make
- Linux Kernel Headers
- Node.js
- npm

# 💻 System Requirements

## Hardware

Recommended:

- 64-bit processor
- Minimum 4 GB RAM
- 8 GB RAM or more recommended
- At least 10 GB free storage

A physical TPM chip is **not required** because this project implements a software-based Virtual TPM.

## Operating System

The project is designed to run on:

Linux

Ubuntu is recommended.

If your main computer uses Windows, Ubuntu can be installed inside a VirtualBox virtual machine.

# 🚀 How to Run the Project

## Step 1 — Start Ubuntu

Open your Ubuntu/Linux environment and launch the Terminal.

# Step 2 — Install Required Packages

Update the package list:

sudo apt update

Install development tools:

sudo apt install build-essential git

Install Linux kernel headers:

sudo apt install linux-headers-$(uname -r)

Verify the kernel version:

uname -r

# Step 3 — Clone the Repository

Clone the GitHub repository:

git clone https://github.com/Ahinsha9/secure-vtpm.git

Enter the project directory:

cd secure-vtpm

Check the project structure:

ls

You should see directories similar to:

client
data
driver
frontend
src
vtpm

# Step 4 — Build the Linux Driver

Go to the driver directory:

cd driver

Check the files:

ls

Build the driver:

make

After a successful build, a Linux kernel module with a `.ko` extension should be generated.

For example:

<driver-name>.ko

# Step 5 — Load the Driver

Load the generated kernel module:

sudo insmod <driver-name>.ko

Replace `<driver-name>.ko` with the actual `.ko` file generated by your driver.

Verify that the driver has loaded:

lsmod | grep vtpm

Check the kernel messages:

dmesg | tail -n 30

# Step 6 — Check the Virtual TPM Device

The driver should create the virtual TPM device:

/dev/vtpm_secure

Check it:

ls -l /dev/vtpm_secure

The device should display its owner, group and permissions.

For example:

crw-rw---- 1 root vtpm ...

This indicates that access to the device is restricted using Linux permissions.

# Step 7 — Configure vTPM User Permissions

If the `vtpm` group does not already exist:

sudo groupadd vtpm

Add your current user to the group:

sudo usermod -aG vtpm $USER

Log out and log back into Ubuntu for the group change to take effect.

Verify:

groups

You should see:

vtpm

Check the device again:

ls -l /dev/vtpm_secure

# Step 8 — Start the Virtual TPM

Return to the project root:

cd ..

Make sure you are in:

secure-vtpm/

Start the vTPM:

./vtpm

If execution permission is missing:

chmod +x vtpm

Then:

./vtpm

Keep this terminal running while using the client.

# Step 9 — Run the vTPM Client

Open a **second terminal**.

Go to the project:

cd secure-vtpm

Enter the client directory:

cd client

Run the client:

./vtpm_cli

The client communicates with the Virtual TPM through the Linux device interface.

If a permission error occurs, first verify:

groups

and:

ls -l /dev/vtpm_secure

Only use `sudo ./vtpm_cli` if your project's permissions require it.

# Step 10 — Start the Frontend

Open a **third terminal**.

Go to the frontend:

cd secure-vtpm/frontend

Install the frontend dependencies:

npm install

Start the development server:

npm run dev

The terminal will display a local URL.

For example:

http://localhost:5173

Open the displayed URL in your browser.

# ▶️ Complete Startup Order

For the project demonstration, use the following order.

## Terminal 1 — Linux Driver

cd secure-vtpm/driver
make
sudo insmod <driver-name>.ko

Verify:

ls -l /dev/vtpm_secure

## Terminal 2 — vTPM

cd secure-vtpm
./vtpm

Keep this terminal running.

## Terminal 3 — Client

cd secure-vtpm/client
./vtpm_cli

## Terminal 4 — Frontend

cd secure-vtpm/frontend
npm install
npm run dev

Open the URL shown by the frontend.

# 🔄 Complete Execution Flow

                    USER
                     │
                     ▼
              ┌─────────────┐
              │  FRONTEND   │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │  vtpm_cli   │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │    vTPM     │
              │   ./vtpm    │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │    Linux    │
              │    Driver   │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │/dev/vtpm_   │
              │   secure    │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ Secure Key  │
              │   Storage   │
              └─────────────┘

# 🔍 Verify the System

Before demonstrating the project, verify each component.

### Check the driver

lsmod | grep vtpm

### Check the device

ls -l /dev/vtpm_secure

### Check user permissions

groups

### Check the vTPM process

ps aux | grep vtpm

### Check the frontend

Open the URL provided by:

npm run dev

# 🔐 Key Management

Cryptographic key material is stored locally under:

data/keys/

Private keys should **never be uploaded to GitHub**.

The repository's `.gitignore` excludes generated PEM key files.

Before committing changes, always check:

git status

Make sure private keys, credentials or other sensitive files are not being committed.

# 🔒 Security Design

The project demonstrates several security concepts.

## 1. Restricted Device Access

The virtual TPM device:

/dev/vtpm_secure

can be protected using Linux ownership, groups and permissions.

## 2. Kernel/User-Space Separation

The project separates the low-level Linux driver from userspace applications:

Kernel Space
     │
     │ Driver
     ▼
Virtual Device
     ▲
     │
Userspace
     │
     ├── vTPM
     └── Client

## 3. Key Isolation

Private cryptographic keys are kept in the local key-storage directory and are not intended to be displayed through the frontend.

## 4. GitHub Security

Private keys and generated sensitive files should not be committed to the public repository.

# 🧪 Testing

The following tests can be performed during project demonstration.

### Test 1 — Driver Loading

lsmod | grep vtpm

Expected result: the vTPM driver/module appears.

### Test 2 — Device Creation

ls -l /dev/vtpm_secure

Expected result:

/dev/vtpm_secure

exists.

### Test 3 — Permission Verification

groups

Expected result: the user belongs to the required `vtpm` group.

### Test 4 — vTPM Startup

./vtpm

Expected result: the vTPM starts without initialization errors.

### Test 5 — Client

./vtpm_cli

Expected result: the client starts and communicates with the vTPM.

### Test 6 — Frontend

npm run dev

Expected result: the frontend starts and can be opened in a browser.

# 🛑 Stopping the Project

Stop the frontend:

Ctrl + C

Stop the vTPM:

Ctrl + C

If the driver needs to be unloaded:

sudo rmmod <driver-name>

Verify:

lsmod | grep vtpm

# 🛠️ Troubleshooting

## `/dev/vtpm_secure` does not exist

Check whether the driver is loaded:

lsmod | grep vtpm

Check kernel messages:

dmesg | tail -n 50

## Permission denied

Check:

groups

and:

ls -l /dev/vtpm_secure

Make sure your user belongs to the appropriate group.

## Driver fails to load

Check:

dmesg | tail -n 50

Verify the running kernel:

uname -r

Make sure the corresponding Linux kernel headers are installed.

## `./vtpm` permission denied

Run:

```bash
chmod +x vtpm
```

Then:

./vtpm

## `./vtpm_cli` permission denied

Run:

chmod +x vtpm_cli

Then:

./vtpm_cli

If the problem is related to the device rather than executable permissions, check:

ls -l /dev/vtpm_secure

## Frontend does not start

Go to:

cd secure-vtpm/frontend

Install dependencies:

npm install

Then:

npm run dev

# ⚠️ Security Considerations

This project is intended for **academic and educational purposes**.

It should not be considered a replacement for a certified hardware TPM.

Do not use this simulator to protect real production credentials or critical secrets.

Never commit:

- Private keys
- Passwords
- API tokens
- SSH private keys
- Authentication credentials

to GitHub.

# ⚠️ Limitations

This project is a software-based Virtual TPM simulator.

Unlike a physical TPM:

- It does not provide physical hardware isolation.
- Key storage is software-based.
- It is not a certified TPM implementation.
- It has not undergone a formal security audit.
- It should not be used as a production hardware-security replacement.

# 🔮 Future Enhancements

Possible future improvements include:

- Full TPM 2.0 command compatibility
- PCR simulation
- Secure boot measurement simulation
- Remote attestation
- Improved cryptographic operations
- Encrypted persistent key storage
- Improved audit logging
- Role-based access control
- Automated security testing
- Fuzz testing
- CI/CD integration
- Enhanced frontend visualization
- Hardware TPM integration

# 🎓 Project Demonstration

For an academic demonstration, the recommended sequence is:

### 1. Start Ubuntu

Ubuntu/Linux

### 2. Load the driver

sudo insmod <driver-name>.ko

### 3. Show the device

ls -l /dev/vtpm_secure

### 4. Explain permissions

Show the `root:vtpm` ownership and restricted permissions.

### 5. Start vTPM

./vtpm

### 6. Start the client

./vtpm_cli

### 7. Start the frontend

npm run dev

### 8. Demonstrate the workflow

Explain:

User
 ↓
Frontend
 ↓
Client
 ↓
vTPM
 ↓
Linux Driver
 ↓
Virtual Device
 ↓
Secure Key Storage

# 📌 Repository

GitHub Repository:

https://github.com/Ahinsha9/secure-vtpm

# 👨‍💻 Author

Ahinsha Das
GitHub:

https://github.com/Ahinsha9

# 📜 Disclaimer

This project is developed for **academic, educational and research purposes**.

The Secure Hardware Key Enclave Simulator demonstrates Virtual TPM and secure hardware-key-enclave concepts in software. It should not be considered equivalent to a certified hardware TPM or used as a production security solution.

# ⭐ Project Summary

The **Secure Hardware Key Enclave Simulator — Virtual TPM** demonstrates how a software-based trusted execution and key-management environment can be constructed using Linux.

The project combines:

Linux
  +
Kernel Driver
  +
Virtual Device
  +
Virtual TPM
  +
Cryptographic Key Management
  +
Client
  +
Web Frontend

to provide a practical demonstration of secure hardware key-enclave concepts.
