/**
* Copyright 2022 AntGroup CO., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

//
// Created by botu.wzy
//

#pragma once
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <utility>

namespace server {
class Service {
   std::string service_name_;
   std::string pid_path_;

   static const int NO_PID_FILE = -1;
   static const int ACCESS_FAILED = -2;
   static const int OTHER_ERROR = -3;
   static const int ALREADY_STARTED = -4;
   /**
    * Gets service PID if the service exists, otherwise return NO_PID_FILE.
    *
    * \return  The service PID.
    *          NO_PID_FILE if the service does not exist.
    *          ACCESS_FAILED if failed to access the pid file.
    */
   [[nodiscard]] int GetServicePid() const {
       if (std::filesystem::exists(pid_path_)) {
           std::ifstream pid_file(pid_path_);
           if (!pid_file.good()) return -2;
           std::string line;
           std::getline(pid_file, line);
           return std::stoi(line);
       } else {
           return -1;
       }
   }

   /**
    * Sets service PID
    *
    * \param pid   The PID.
    *
    * \return  0 if success. ACCESS_FAILED if cannot access the file.
    */
   [[nodiscard]] int SetServicePid(int pid) const {
       std::ofstream pid_file(pid_path_);
       if (!pid_file.good()) return ACCESS_FAILED;
       pid_file << pid;
       return 0;
   }

   /**
    * Removes the service PID file
    * \return  0 if success. ACCESS_FAILED if cannot access the file.
    */
   [[nodiscard]] int RemoveServicePidFile() const {
       if (!std::filesystem::remove(pid_path_)) return ACCESS_FAILED;
       return 0;
   }

  public:
   /**
    * Constructor a service instance.
    *
    * \param service_name  Name of the service. This will be used to identify the service.
    *                      If the service name is already used by another program, Start()
    *                      will issue a warning and exit, and Stop() will actually stop
    *                      the existing program.
    */
   Service(std::string  service_name, std::string  pid_path)
       : service_name_(std::move(service_name)), pid_path_(std::move(pid_path)) {}

   /**
    * Starts the service.
    *
    *   On Windows, this just calls Run().
    *   On Linux, the process will first check if the service is already running. If not,
    *   it spawns a child process which in turn calls Run(), and the parent process will exit.
    *
    * \return  0 on success. -1 on failure.
    */
   int Start() {
       LOG_INFO("Starting {} ...", service_name_);
       // check service availability
       int oldpid = GetServicePid();
       // failed to access the pid file, probably access right problem
       if (oldpid == ACCESS_FAILED) {
           LOG_WARN("Failed to open pid file. Make sure you are running as root.");
           return ACCESS_FAILED;
       }
       // service already exist
       if (oldpid != NO_PID_FILE && ProcessRunning(oldpid)) {
           LOG_WARN("Service {} already exists at pid {}. You can stop it and start again.", service_name_, oldpid);
           return ALREADY_STARTED;
       }
       // try and set pid file to check if we have correct access right
       if (SetServicePid(-1) != 0) {
           LOG_WARN("Failed to write pid file. Make sure you are running as root. ");
           return ACCESS_FAILED;
       }
       std::filesystem::path currentPath = std::filesystem::current_path();
       // fork child process
       int pid = fork();
       if (pid == -1) {
           LOG_WARN("Failed to create child process: {}", strerror(errno));
           return OTHER_ERROR;
       }
       if (pid > 0) {
           // this is parent
           LOG_INFO("The service process is started at pid {}.", pid);
           // write pid to the pid file
           if (SetServicePid(pid) != 0) {
               LOG_WARN("Failed to write pid file. Make sure you are running as root. ");
               return ACCESS_FAILED;
           }
           return 0;
       } else {
           int sid = setsid();
           if (sid < 0) {
               LOG_WARN("Failed to create session id: {}", strerror(errno));
               return OTHER_ERROR;
           }
           if ((chdir(currentPath.c_str())) < 0) {
               LOG_WARN("Failed to chdir: {}", strerror(errno));
               return OTHER_ERROR;
           }
           int fd = open("/dev/null", O_RDWR);
           dup2(fd, 0);
           dup2(fd, 1);
           dup2(fd, 2);
           if (fd > 2) close(fd);
           // set process name
           prctl(PR_SET_NAME, service_name_.c_str());
           return Run();
       }
   }

   /**
    * Stops the service.
    *   On Windows, this does nothing.
    *   On Linux, it stops the service by sending a kill signal.
    *
    * \return  0 on success. -1 on error.
    */
   int Stop() {
       LOG_INFO("Stopping {}...", service_name_);
       int pid = GetServicePid();
       if (pid == ACCESS_FAILED) {
           LOG_WARN("Failed to read pid file. Make sure you are running as root.");
           return ACCESS_FAILED;
       }
       if (pid == NO_PID_FILE || !ProcessRunning(pid)) {
           LOG_WARN("Service {} is already dead.", service_name_);
           return NO_PID_FILE;
       }
       int r = kill(pid, SIGUSR1);
       if (r == EPERM) {
           LOG_WARN("Failed to kill service {}:{}", service_name_, strerror(errno));
           return OTHER_ERROR;
       }
       LOG_INFO("Process stopped.");
       waitpid(pid, 0, 0);
       if (RemoveServicePidFile() != 0) {
           LOG_WARN("Failed to read pid file. Make sure you are running as root.");
           return ACCESS_FAILED;
       }
       return 0;
   }

   int Restart() {
       int r = Stop();
       if (r != 0 && r != NO_PID_FILE) {
           LOG_WARN("Failed to stop existing service.");
           return r;
       }
       return Start();
   }

   virtual int Run() = 0;

  private:
   static bool ProcessRunning(int pid) {
       return kill(pid, 0) == 0;
   }
};
}  // namespace server
