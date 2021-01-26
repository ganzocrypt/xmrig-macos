/* XMRig
 * Copyright 2021-2021 Tony Butler                  <https://github.com/Spudz76>
 * Copyright 2021-2021 XMRig                        <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "backend/cpu/wrappers/HwmonLib.h"
#include "backend/cpu/CpuLaunchData.h"


#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_CPUS 4095


namespace xmrig {


bool HwmonLib::m_initialized          = false;
bool HwmonLib::m_ready                = false;
static const std::string kPrefixCpu   = "/sys/devices/system/cpu/";
static const std::string kPrefixHwmon = "/sys/class/hwmon/";


static inline bool sysfs_is_file(const std::string &path)
{
    struct stat sb;

    return stat(path.c_str(), &sb) == 0 && ((sb.st_mode & S_IFMT) == S_IFREG);
}


uint32_t sysfs_read(const std::string &path)
{
    if (!sysfs_is_file(path)) {
        return 0;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return 0;
    }

    uint32_t value = 0;
    file >> value;

    return value;
}


static inline std::string sysfs_read_str(const std::string &path)
{
    if (!sysfs_is_file(path)) {
        return "";
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::string data;
    std::getline(file, data);

    return data;
}


static inline bool sysfs_is_cpu_core(const std::string &prefix, const uint32_t core)
{
    const std::string path = prefix + "topology/core_id";
    return sysfs_read(path) == core;
}


static inline bool sysfs_is_hwmon(const std::string &path, const std::string &type)
{
    return sysfs_read_str(path) == type;
}


static inline std::string sysfs_prefix_cpu_core(const uint32_t core)
{
    const std::string path = kPrefixCpu + "cpu";

    for (uint32_t i = 1; i < MAX_CPUS; ++i) {
        const std::string prefix = path + std::to_string(i) + "/";
        if (sysfs_is_cpu_core(prefix, core)) {
            return prefix;
        }
    }

    return {};
}


static inline std::string sysfs_prefix_hwmon(const std::string &type)
{
    const std::string path = kPrefixHwmon + "hwmon";

    for (uint32_t i = 1; i < 100; ++i) {
        const std::string prefix = path + std::to_string(i) + "/";
        if (sysfs_is_hwmon(prefix + "name", type)) {
            return prefix;
        }
    }

    return {};
}


} // namespace xmrig


bool xmrig::HwmonLib::init()
{
    if (!m_initialized) {
        m_ready       = dlopen() && load();
        m_initialized = true;
    }

    return m_ready;
}


const char *xmrig::HwmonLib::lastError() noexcept
{
    return nullptr;
}


void xmrig::HwmonLib::close()
{
}


HwmonHealth xmrig::HwmonLib::health()
{
    if (!isReady()) {
        return {};
    }

    HwmonHealth health;
    health.clock        = 2200; //sysfs_read(prefix_coretemp + "freq1_input") / 1000000;
    health.memClock     = 1600; //sysfs_read(prefix_coretemp + "freq2_input") / 1000000;
    health.power        =   75; //sysfs_read(prefix_coretemp + "power1_average") / 1000000;
    health.rpm          = 3249; //sysfs_read(prefix_coretemp + "fan1_input");

    uint32_t core = 0;
    for (uint32_t i = 0; i < MAX_CPUS; ++i) {
        const auto prefix = sysfs_prefix_cpu_core(i);
        if (prefix.empty()) {
            break;
        } else {
            const std::string prefix_cpufreq = prefix + "cpufreq/";
            const std::string path = prefix_cpufreq + "scaling_driver";
            std::string driver = sysfs_read_str(path);
            if (driver == "intel_pstate") {
                health.clock = sysfs_read(prefix_cpufreq + "scaling_cur_freq") / 1000;
            }

            core++;
        }
    }

    const auto prefix_coretemp = sysfs_prefix_hwmon("coretemp");
    if (prefix_coretemp.empty()) {
        return {};
    }

    health.temperature  = sysfs_read(prefix_coretemp + "temp1_input") / 1000;

    const auto prefix_thinkpad = sysfs_prefix_hwmon("thinkpad");
    if (!prefix_thinkpad.empty()) {
        health.rpm = sysfs_read(prefix_thinkpad + "fan1_input");
    }

    if (!health.temperature) {
        health.temperature = 69;
    }

    return health;
}


bool xmrig::HwmonLib::dlopen()
{
    struct stat sb;
    if (stat(kPrefixCpu.c_str(), &sb) == -1) {
        return false;
    }

    return (sb.st_mode & S_IFMT) == S_IFDIR;
}


bool xmrig::HwmonLib::load()
{
    return true;
}
