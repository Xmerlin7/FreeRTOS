# FreeRTOS Fundamentals Project

## Overview 🚀

Welcome to the FreeRTOS Fundamentals project! This small-scale project that I used a mix APIs of **native freeRTOS and CMSIS** delves into the core elements of FreeRTOS, offering a hands-on exploration of key concepts. Let's dive into the fundamentals:

## Project Highlights 🧾

### Semaphores 🔒

- Binary Semaphore : 
ensures protection for the `Shared_Resource` variable. To aovid the context switch in non-entrant functions;

- Counting Semaphore :
Used to Synchronize tasks.

### Queue Communication 📬

 Tasks communicate and share info, teaming up smoothly through inter-task communication for a safe data system. 🔄🤝

### Software Periodic Timer ⏰

Experience the precision of a software timer as it orchestrates periodic actions. Witness the overwrite of the `Shared_Resource` variable every **10 seconds** by a `Callback01` function.

### UART Communication 📡

Embark on a journey into UART communication, where real-time data is displayed through the RealTerm terminal. For this project, the STM32F401RCT6 microcontroller is utilized, with USB UART providing the communication link to RealTerm. This visual representation enhances the project's user interface, providing dynamic insights into the system's behavior.

## Usage 🛠

1. Clone the repository.
2. Set up your development environment with FreeRTOS.
3. Build and flash the project to your STM32F401RCT6 target device.
4. Connect USB UART for RealTerm communication.
5. Observe the RealTerm terminal to witness the dynamic behavior of the system.

## Contributing 🤝

Contributions are not just welcome; they're encouraged! Feel free to submit issues or pull requests to enhance the fundamental understanding of FreeRTOS in this project.

## Contact 📬

For any questions or suggestions, reach out through:

  <a href="https://www.linkedin.com/in/seif-eldarageely-a27125227/?trk=public-profile-join-page" target="_blank">
    <img src="https://img.shields.io/static/v1?message=LinkedIn&logo=linkedin&label=&color=0077B5&logoColor=white&labelColor=&style=for-the-badge" height="35" alt="linkedin logo"  />
  </a>

Happy coding! 🚀
