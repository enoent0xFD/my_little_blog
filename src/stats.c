// src/stats.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>
#elif __linux__
#include <sys/sysinfo.h>
#endif

#include "../include/stats.h"

static time_t start_time;

void init_stats(void) { start_time = time(NULL); }

static void format_time(char *buffer, size_t size) {
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(buffer, size, "%H:%M:%S", tm_info);
}

static void format_memory(char *buffer, size_t size) {
#ifdef __APPLE__
  struct mach_task_basic_info info;
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info,
                &count) == KERN_SUCCESS) {
    // Convert to MB
    unsigned long resident_mb = info.resident_size / (1024 * 1024);
    unsigned long virtual_mb = info.virtual_size / (1024 * 1024);
    snprintf(buffer, size, "%luMB", resident_mb, virtual_mb);
  } else {
    strncpy(buffer, "N/A", size);
  }
#elif __linux__
  FILE *fp = fopen("/proc/self/status", "r");
  unsigned long vmsize = 0, vmrss = 0;

  if (fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      if (strncmp(line, "VmSize:", 7) == 0) {
        sscanf(line, "VmSize: %lu", &vmsize);
      }
      if (strncmp(line, "VmRSS:", 6) == 0) {
        sscanf(line, "VmRSS: %lu", &vmrss);
      }
    }
    fclose(fp);
    // Convert KB to MB
    vmsize /= 1024;
    vmrss /= 1024;
    snprintf(buffer, size, "%luMB", vmrss);
  } else {
    strncpy(buffer, "N/A", size);
  }
#else
  strncpy(buffer, "N/A", size);
#endif
}

static void get_os_info(char *buffer, size_t size) {
#ifdef __APPLE__
  char os_version[256] = {0};
  size_t len = sizeof(os_version);
  sysctlbyname("kern.osproductversion", &os_version, &len, NULL, 0);
  snprintf(buffer, size, "macOS %s", os_version);
#elif __linux__
  FILE *fp = fopen("/etc/os-release", "r");
  if (fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
        char *name = strchr(line, '"');
        if (name) {
          name++; // skip first quote
          char *end = strchr(name, '"');
          if (end)
            *end = '\0';
          strncpy(buffer, name, size);
          fclose(fp);
          return;
        }
      }
    }
    fclose(fp);
  }
  strncpy(buffer, "Linux", size);
#else
  strncpy(buffer, "Unknown OS", size);
#endif
}

struct system_stats get_system_stats(void) {
  struct system_stats stats;

  format_time(stats.uptime, sizeof(stats.uptime));
  format_memory(stats.memory, sizeof(stats.memory));
  get_os_info(stats.os_info, sizeof(stats.os_info));

  return stats;
}
