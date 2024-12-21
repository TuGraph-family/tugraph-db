//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <atomic>
#include <cstring>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <regex>

#ifdef _WIN32
#include <intrin.h>
#include <array>
#include <bitset>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <Assert.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#include <tchar.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#include <psapi.h>
#elif __APPLE__  // #ifdef _WIN32 #elif __APPLE__
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>
#include <fstream>
#else  // #ifdef _WIN32 #elif __APPLE__
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/times.h>
#if !USING_CENTOS9
#include <sys/vtimes.h>
#endif
#include <limits.h>
#include <fstream>
#endif  // #ifdef _WIN32 #elif __APPLE__

#include "fma-common/string_util.h"
#include "fma-common/timed_task.h"
#include "fma-common/utils.h"

namespace fma_common {
namespace HardwareInfo {

// by qinwei
struct CPURate {
    CPURate() : selfCPURate(0), serverCPURate(0) {}
    CPURate(double self, double server) : selfCPURate(self), serverCPURate(server) {}
    CPURate(const CPURate& rhs) = default;

    double selfCPURate;
    double serverCPURate;
};

// by qinwei
struct MemoryInfo {  // unit: KB
    uint64_t total;
    uint64_t free;
    uint64_t available;
    uint64_t used;
    uint64_t shared;
    uint64_t buff;
    uint64_t cache;
    uint64_t selfMemory;
    uint64_t dwMemoryLoad;  // percentage of physical memory that is in use
};

inline unsigned int GetCpuCount() { return std::thread::hardware_concurrency(); }

#ifdef _WIN32
inline std::string GetCpuId() {
    int regs[4] = {0};
    std::string vendor(12, 0);
    __cpuid(regs, 0);                 // mov eax,0; cpuid
    memcpy(&vendor[0], &regs[1], 4);  // copy EBX
    memcpy(&vendor[4], &regs[3], 4);  // copy ECX
    memcpy(&vendor[8], &regs[2], 4);  // copy EDX
    return vendor;
}

inline std::string GetCpuModel() {
    std::array<int, 4> cpui;
    int nExIds_;
    std::vector<std::array<int, 4>> extdata_;
    __cpuid(cpui.data(), 0x80000000);
    nExIds_ = cpui[0];

    std::string brand(0x40, 0);

    for (int i = 0x80000000; i <= nExIds_; ++i) {
        __cpuidex(cpui.data(), i, 0);
        extdata_.push_back(cpui);
    }

    // Interpret CPU brand string if reported
    if (nExIds_ >= 0x80000004) {
        memcpy(&brand[0], extdata_[2].data(), sizeof(cpui));
        memcpy(&brand[16], extdata_[3].data(), sizeof(cpui));
        memcpy(&brand[32], extdata_[4].data(), sizeof(cpui));
    }
    return brand.data();
}

inline std::set<std::string> ListMacAddr() {
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(AdapterInfo);
    std::string mac_addr(17, 0);
    std::set<std::string> ret;
    AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL) {
        return ret;
    }
    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
        if (AdapterInfo == NULL) {
            return ret;
        }
    }
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;  // Contains pointer to current adapter info
        do {
            std::ostringstream oss;
            for (int i = 0; i < 6; i++) {
                oss << std::hex << std::setfill('0') << std::setw(2)
                    << (int)pAdapterInfo->Address[i];
                if (i != 5) oss << ":";
            }
            ret.emplace(oss.str());
            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    }
    free(AdapterInfo);
    return ret;
}

inline std::string GetHostName() {
    static const int INFO_BUFFER_SIZE = 32767;
    TCHAR infoBuf[INFO_BUFFER_SIZE + 1];
    DWORD bufCharCount = INFO_BUFFER_SIZE;
    if (!GetComputerName(infoBuf, &bufCharCount))
        throw std::runtime_error("Error trying to get hostname.");
    return std::string(infoBuf);
}

inline size_t GetAvailableMemory() {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
}

// by qinwei
inline void GetMemoryInfo(struct MemoryInfo& memoryInfo) {
    memoryInfo.total = -1;
    memoryInfo.free = -1;
    memoryInfo.available = -1;
    memoryInfo.used = -1;
    memoryInfo.shared = 0;
    memoryInfo.buff = 0;
    memoryInfo.cache = 0;
    memoryInfo.selfMemory = 0;
    memoryInfo.dwMemoryLoad = 0;

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    memoryInfo.total = memInfo.ullTotalPhys / 1024;
    memoryInfo.available = memInfo.ullAvailPhys / 1024;
    memoryInfo.free = memoryInfo.available;
    memoryInfo.dwMemoryLoad = memInfo.dwMemoryLoad;
    memoryInfo.used = memoryInfo.total * memoryInfo.dwMemoryLoad / 100;

    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    memoryInfo.selfMemory = pmc.WorkingSetSize / 1024;
}
#else
inline std::string GetCpuId() {
#ifdef __aarch64__
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (cpuinfo.good()) {
        std::getline(cpuinfo, line);
        if (StartsWith(line, "vendor_id")) {
            break;
        }
    }
    auto parts = Split(line, ":");
    if (parts.size() != 2) return "";
    return Strip(parts[1], " \t");
#else
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    eax = 0;
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "0"(eax), "2"(ecx)
                 : "memory");
    std::string vendor(12, 0);
    memcpy(&vendor[0], &ebx, 4);
    memcpy(&vendor[4], &edx, 4);
    memcpy(&vendor[8], &ecx, 4);
    return vendor;
#endif
}

inline std::string GetCpuModel() {
#ifdef __APPLE__
    std::stringstream ss;
    std::string cmd = "sysctl -a | grep brand_string";
    if (ExecCmdThroughPopen(cmd, ss) < 0)
        throw std::runtime_error("Error trying to get cpu model.");
    std::string line;
    while (ss.good()) {
        std::getline(ss, line);
        if (StartsWith(line, "machdep.cpu.brand_string")) {
            break;
        }
    }
    auto parts = Split(line, ":");
    if (parts.size() != 2) return "";
    return Strip(parts[1], " ");
#elif __aarch64__
    std::stringstream ss;
    std::string cmd = "lscpu";
    ExecCmdThroughPopen(cmd, ss);
    std::string line;
    while (ss.good()) {
        std::getline(ss, line);
        if (StartsWith(line, "Model name") || StartsWith(line, "型号名称")) {
            break;
        }
    }
    std::vector<std::string> parts;
    if (StartsWith(line, "Model name")) {
        parts = Split(line, ":");
    } else if (StartsWith(line, "型号名称")) {
        parts = Split(line, "：");
    }
    if (parts.size() < 2) return "";
    return Strip(parts[parts.size() - 1], " ");
#else
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (cpuinfo.good()) {
        std::getline(cpuinfo, line);
        if (StartsWith(line, "model name")) {
            break;
        }
    }
    auto parts = Split(line, ":");
    if (parts.size() != 2) return "";
    return Strip(parts[1], " \t");
#endif
}

inline std::set<std::string> ListMacAddr() {
#ifdef __APPLE__
    // TODO(hk)
    std::set<std::string> ret;
    return ret;
#else
    struct ifaddrs* ifaddr = NULL;
    struct ifaddrs* ifa = NULL;
    int i = 0;
    std::set<std::string> ret;
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if ((ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) &&
                !(ifa->ifa_flags & IFF_LOOPBACK)) {
                struct sockaddr_ll* s = (struct sockaddr_ll*)ifa->ifa_addr;

                std::ostringstream oss;
                for (i = 0; i < s->sll_halen; i++) {
                    oss << std::hex << std::setfill('0') << std::setw(2) << (int)s->sll_addr[i];
                    if (i != s->sll_halen - 1) oss << ":";
                }
                ret.insert(oss.str());
            }
        }
        freeifaddrs(ifaddr);
    }
    return ret;
#endif
}

inline std::string GetHostName() {
    char hostname[1024];
    if (gethostname(hostname, 1023)) {
        throw std::runtime_error("Error trying to get hostname.");
    }
    return std::string(hostname);
}

inline size_t GetAvailableMemory() {
#ifdef __APPLE__
    // TODO(hk)
    return 0;
#else
    int64_t pages = sysconf(_SC_AVPHYS_PAGES);
    int64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#endif
}

// by qinwei
inline void _UpdateCPURate(struct CPURate& cpuRate, int interval = 200) {
    cpuRate.selfCPURate = 0.0;
    cpuRate.serverCPURate = 0.0;

    int numProcessors = 0;
    uint64_t utime, ntime, stime, itime, iowtime, irqtime, sirqtime;
    uint64_t cputimeLast, cputimeCurr;
    uint64_t cputimeIdelLast, cputimeIdelCurr;
    clock_t cputimeProcLast, cputimeProcCurr;

    FILE* file;
    struct tms timeSample;
    numProcessors = std::thread::hardware_concurrency();

    auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &utime, &ntime, &stime, &itime, &iowtime,
           &irqtime, &sirqtime);
    fclose(file);
    times(&timeSample);
    cputimeProcLast =
        timeSample.tms_stime + timeSample.tms_utime + timeSample.tms_cutime + timeSample.tms_cstime;
    cputimeLast = utime + ntime + stime + itime + iowtime + irqtime + sirqtime;
    cputimeIdelLast = itime;

    std::this_thread::sleep_until(x);

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &utime, &ntime, &stime, &itime, &iowtime,
           &irqtime, &sirqtime);
    fclose(file);
    times(&timeSample);
    cputimeProcCurr =
        timeSample.tms_stime + timeSample.tms_utime + timeSample.tms_cutime + timeSample.tms_cstime;
    cputimeCurr = utime + ntime + stime + itime + iowtime + irqtime + sirqtime;
    cputimeIdelCurr = itime;

    cpuRate.selfCPURate = ((double)cputimeProcCurr - (double)cputimeProcLast) /
                          ((double)cputimeCurr - (double)cputimeLast) * (double)numProcessors *
                          100.0;
    cpuRate.serverCPURate = ((double)cputimeIdelCurr - (double)cputimeIdelLast) /
                            ((double)cputimeCurr - (double)cputimeLast);
    cpuRate.serverCPURate = (1.0 - cpuRate.serverCPURate) * (double)numProcessors * 100.0;
}

// by qinwei
inline uint64_t _ParseMemLine(char* line) {
    size_t i = strlen(line);
    const char* p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    uint64_t res = atol(p);
    return res;
}

// by qinwei
inline void GetMemoryInfo(struct MemoryInfo& memoryInfo) {
#ifdef __APPLE__
    // TODO(hk)
    return;
#endif
    memoryInfo.total = -1;
    memoryInfo.free = -1;
    memoryInfo.available = -1;
    memoryInfo.used = -1;
    memoryInfo.shared = -1;
    memoryInfo.buff = -1;
    memoryInfo.cache = 0;
    memoryInfo.selfMemory = 0;
    memoryInfo.dwMemoryLoad = 0;

    FILE* file = fopen("/proc/meminfo", "r");
    char line[1024];
    while (fgets(line, 1024, file) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0)
            memoryInfo.total = _ParseMemLine(line);
        else if (strncmp(line, "MemFree:", 8) == 0)
            memoryInfo.free = _ParseMemLine(line);
        else if (strncmp(line, "MemAvailable:", 13) == 0)
            memoryInfo.available = _ParseMemLine(line);
        else if (strncmp(line, "Shmem:", 6) == 0)
            memoryInfo.shared = _ParseMemLine(line);
        else if (strncmp(line, "Buffers:", 8) == 0)
            memoryInfo.buff = _ParseMemLine(line);
        else if (strncmp(line, "Cached:", 7) == 0)
            memoryInfo.cache += _ParseMemLine(line);
        else if (strncmp(line, "Slab:", 5) == 0)
            memoryInfo.cache += _ParseMemLine(line);
    }
    memoryInfo.used = memoryInfo.total - memoryInfo.free - memoryInfo.buff - memoryInfo.cache;
    if (memoryInfo.available == ((uint64_t)(-1))) memoryInfo.available = memoryInfo.free;
    fclose(file);

    file = fopen("/proc/self/status", "r");
    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            memoryInfo.selfMemory = _ParseMemLine(line);
            break;
        }
    }
    fclose(file);

    memoryInfo.dwMemoryLoad = (uint64_t)(100 * memoryInfo.used / memoryInfo.total);
}
#endif
class CPURateFetcher {
    std::atomic<double> total_rate_;
    std::atomic<double> self_rate_;

    int n_processors_;
    TimedTaskScheduler::TaskPtr timed_task_;

#ifdef _WIN32
    PDH_HQUERY cpu_query_;
    PDH_HCOUNTER counter_desc_;
    HANDLE process_handle_;
    uint64_t last_cpu_time_total_;
    uint64_t last_cpu_time_proc_;

    void Init() {
        n_processors_ = GetCpuCount();
        PdhOpenQuery(NULL, NULL, &cpu_query_);
        PdhAddEnglishCounter(cpu_query_, TEXT("\\Processor(_Total)\\% Processor Time"), NULL,
                             &counter_desc_);
        process_handle_ = GetCurrentProcess();
        last_cpu_time_total_ = 0;
        last_cpu_time_proc_ = 0;
    }

    void Destroy() { PdhCloseQuery(cpu_query_); }

    void UpdateValues() {
        PdhCollectQueryData(cpu_query_);
        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
        uint64_t curr_cpu_time, curr_cpu_proc_time;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));
        curr_cpu_time = lastCPU.QuadPart;

        GetProcessTimes(process_handle_, &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
        curr_cpu_proc_time = lastSysCPU.QuadPart + lastUserCPU.QuadPart;

        PDH_FMT_COUNTERVALUE counterVal;
        PdhGetFormattedCounterValue(counter_desc_, PDH_FMT_DOUBLE, NULL, &counterVal);

        total_rate_ = counterVal.doubleValue;
        self_rate_ = ((double)curr_cpu_proc_time - (double)last_cpu_time_proc_) /
                     ((double)curr_cpu_time - (double)last_cpu_time_total_) /
                     (double)n_processors_ * 100.0;
        last_cpu_time_proc_ = curr_cpu_proc_time;
        last_cpu_time_total_ = curr_cpu_time;
    }
#else
    uint64_t last_cpu_time_total_;
    uint64_t last_cpu_time_idel_;
    clock_t last_cpu_time_proc_;

    void Init() {
        n_processors_ = GetCpuCount();
        last_cpu_time_total_ = 0;
        last_cpu_time_idel_ = 0;
        last_cpu_time_proc_ = 0;
    }

    void Destroy() {}

    void UpdateValues() {
#ifdef __APPLE__
        // TODO(hk)
        return;
#endif
        uint64_t utime, ntime, stime, itime, iowtime, irqtime, sirqtime;
        uint64_t curr_cpu_time_total;
        uint64_t curr_cpu_time_idel;
        clock_t curr_cpu_time_proc;

        FILE* file;
        file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &utime, &ntime, &stime, &itime,
               &iowtime, &irqtime, &sirqtime);
        fclose(file);
        struct tms time_sample;
        times(&time_sample);
        curr_cpu_time_proc = time_sample.tms_stime + time_sample.tms_utime +
                             time_sample.tms_cutime + time_sample.tms_cstime;
        curr_cpu_time_total = utime + ntime + stime + itime + iowtime + irqtime + sirqtime;
        curr_cpu_time_idel = itime;

        self_rate_ = ((double)curr_cpu_time_proc - (double)last_cpu_time_proc_) /
                     ((double)curr_cpu_time_total - (double)last_cpu_time_total_) * 100.0;
        total_rate_ = (1.0 - (((double)curr_cpu_time_idel - (double)last_cpu_time_idel_) /
                              ((double)curr_cpu_time_total - (double)last_cpu_time_total_))) *
                      100;
        last_cpu_time_total_ = curr_cpu_time_total;
        last_cpu_time_idel_ = curr_cpu_time_idel;
        last_cpu_time_proc_ = curr_cpu_time_proc;
    }
#endif

 public:
    CPURateFetcher() {
        Init();
        UpdateValues();
        timed_task_ = TimedTaskScheduler::GetInstance().ScheduleReccurringTask(
            1000, [this](TimedTask*) { UpdateValues(); });
    }

    ~CPURateFetcher() { Destroy(); }

    CPURate GetCpuRate() {
        return CPURate(self_rate_.load(std::memory_order_acquire),
                       total_rate_.load(std::memory_order_acquire));
    }
};

inline CPURate GetCPURate() {
    static CPURateFetcher rate_fetcher;
    return rate_fetcher.GetCpuRate();
}

struct DiskRate {
    double readRate;
    double writeRate;
};

class DiskRateFetcher {
 private:
    int64_t last_read_count = 0;
    int64_t last_write_count = 0;
    double last_time;

    void get_count(int64_t& read_count, int64_t& write_count) {
        read_count = 0;
        write_count = 0;
#if defined(_WIN32) || defined(__APPLE__)
#else
        std::ifstream fin("/proc/diskstats");
        std::string line;
        std::regex diskfilter("^(dm-\\d+|md\\d+|[hsv]d[a-z]+\\d+)$");
        while (std::getline(fin, line)) {
            auto sline = fma_common::Split(line);
            if (sline[5] == "0" && sline[9] == "0") continue;
            if (sline[1] != "0") continue;
            if (std::regex_match(sline[2], diskfilter)) continue;
            read_count += stol(sline[5]);
            write_count += stol(sline[9]);
        }
        fin.close();
#endif
    }

 public:
    DiskRateFetcher() {
        get_count(last_read_count, last_write_count);
        last_time = GetTime();
    }

    DiskRate get_rate() {
        int64_t curr_read_count = 0;
        int64_t curr_write_count = 0;
        double curr_time = GetTime();
        if (curr_time - last_time < 0.1) {
            SleepUs(100000);
            curr_time = GetTime();
        }

        get_count(curr_read_count, curr_write_count);

        DiskRate dr;
        dr.readRate = (curr_read_count - last_read_count) / (curr_time - last_time) *
                      512;  // Bytes per second
        dr.writeRate = (curr_write_count - last_write_count) / (curr_time - last_time) *
                       512;  // Bytes per second

        last_read_count = curr_read_count;
        last_write_count = curr_write_count;
        last_time = curr_time;
        return dr;
    }
};

inline DiskRate GetDiskRate() {
    static DiskRateFetcher drf;
    return drf.get_rate();
}
}  // namespace HardwareInfo
}  // namespace fma_common
