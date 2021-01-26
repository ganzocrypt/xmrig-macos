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

#ifndef XMRIG_HWMONHEALTH_H
#define XMRIG_HWMONHEALTH_H


#include <cstdint>
#include <vector>


struct HwmonHealth
{
    uint32_t clock          = 0;
    uint32_t memClock       = 0;
    uint32_t power          = 0;
    uint32_t rpm            = 0;
    uint32_t temperature    = 0;
};


#endif /* XMRIG_HWMONHEALTH_H */
