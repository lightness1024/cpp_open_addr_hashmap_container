https://sourceforge.net/p/cgenericopenaddresshashmap/

this is an open address hash map
https://en.wikipedia.org/wiki/Open_addressing

hashmap.hpp under zlib license, copyright Vivien Oddou, 2014/2015.
other files are under wtfpl http://www.wtfpl.net/

except winqpc.cpp which is under creative commons SA 3.0
and portions of 'example.cpp' (testXX() functions) that comes from GNU, which are under GPLv3.

benchmark under gcc 4.9 (linux) AMD 6800k

== 1 million int pushes ==
std vector:             13.5705 ms
reserved vector:        10.6002 ms
*open address:          266.702 ms
*reserved openaddr:     104.992 ms
std unordered:          273.966 ms
std map:                706.413 ms

== 100k random erasures ==
*openaddr:              8.72512 ms
std unordered:          21.1096 ms
std map:                71.8747 ms

== 1M iteration ==
*openaddr:              181.742 ms
std unordered:          989.945 ms
std map:                1707.21 ms

== 50k random finds among 1M contenance ==
*openaddr:              4.24008 ms
std unordered:          10.4447 ms
std map:                35.7247 ms



visual studio 2013 win8

== 1 million int pushes ==
std vector:             23.5943 ms
reserved vector:        18.3602 ms
*open address:          79.221 ms
*reserved openaddr:     94.3609 ms
std unordered:          64.4805 ms
std map:                183.308 ms

== 100k random erasures ==
*openaddr:              8.96476 ms
std unordered:          6.2568 ms
std map:                24.5041 ms

== 1M iteration ==
*openaddr:              12.3762 ms
std unordered:          3.81578 ms
std map:                10.6123 ms

== 50k random finds among 1M contenance ==
*openaddr:              3.53564 ms
std unordered:          2.22053 ms
std map:                8.602 ms