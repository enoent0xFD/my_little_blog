// include/stats.h
#ifndef STATS_H
#define STATS_H

struct system_stats {
    char uptime[32];
    char memory[32];
    char os_info[64];
};

// Initialize stats tracking
void init_stats(void);

// Get current system stats
struct system_stats get_system_stats(void);

#endif
