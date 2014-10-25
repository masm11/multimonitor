/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007,2014 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef TYPES_H__INCLUDED
#define TYPES_H__INCLUDED

enum {
    TYPE_BATT_0,
    TYPE_BATT_1,
    
    TYPE_CPUFREQ_0,
    TYPE_CPUFREQ_1,
    TYPE_CPUFREQ_2,
    TYPE_CPUFREQ_3,
    
    TYPE_LOADAVG_1,
    TYPE_LOADAVG_5,
    TYPE_LOADAVG_15,
    
    TYPE_CPULOAD_0,
    TYPE_CPULOAD_1,
    TYPE_CPULOAD_2,
    TYPE_CPULOAD_3,
    
    TYPE_NET_ETH0,
    TYPE_NET_ETH1,
    TYPE_NET_ETH2,
    TYPE_NET_EN,
    TYPE_NET_WLAN0,
    TYPE_NET_ATH0,
    TYPE_NET_WL,
    TYPE_NET_LO,
    
    TYPE_MEM,
    
    TYPE_SWAP,
    
    TYPE_DISK_SDA,
    TYPE_DISK_SDB,
    TYPE_DISK_SDC,
    TYPE_DISK_HDA,
    TYPE_DISK_HDB,
    TYPE_DISK_HDC,
    
    TYPE_TEMP,
    
    TYPE_NR
};

#endif	/* ifndef TYPES_H__INCLUDED */
