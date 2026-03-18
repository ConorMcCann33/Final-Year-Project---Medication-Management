# Medication Management System
Developed as part of a final year engineering project.
This project presents the design and development of a cloud-connected medication management system. The system consists of a physical dispensing device controlled by an ESP32, a mobile application for remote monitoring, and a Firebase Realtime Database for communication.

The system automates medication dispensing, provides user alerts, and allows caregivers to monitor adherence remotely.

## System Overview
The system consists of four main components:

- **ESP32 Embedded System** – Controls dispensing, alerts, and system logic
- **Nextion Display** – Provides a touchscreen user interface
- **Firebase RTDB** – Handles cloud data storage and communication
- **Android Application** – Allows caregivers to monitor system activity

## Features
- Automated medication dispensing
- Configurable daily alarms
- Real-time cloud synchronisation
- Medication adherence tracking
- Push notifications to caregivers
- Historical log of medication events

## Repository Structure

- `/ESP32_Code/FYP_ESP32.ino` – Main embedded firmware
- `/ESP32_Code/StepperTest.ino` – Stepper motor test file
- `/Android_App` – Mobile application
- `/Notification_Function.js` – Cloud notification function
- `/Nextion_Display` – UI project files

## Setup Instructions

### Android App
- Install `app-debug.apk` on an Android device

### ESP32
- Upload the firmware using Arduino IDE
- Configure Wi-Fi credentials

### Firebase
- A valid Firebase project and configuration file (`google-services.json`) is required (not included for security reasons)

## Demonstration Videos

- Successful dispensing operation: [link]
- Alert system operation: [link]
- Full System Operation: [link]

## Technologies Used

- ESP32 (Embedded C++)
- Android Studio (Java/Kotlin)
- Firebase Realtime Database
- Firebase Cloud Functions
- Nextion Display Editor

## Notes
- Firebase configuration file (`google-services.json`) and the `FIREBASE_SECRET` have been excluded for security reasons.

## Author
Conor McCann
Final Year Project – Dublin City University
