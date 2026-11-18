One run
-------

incoming_gal is the source: protocol.gal, console.gal, and one file per aggregate.
Each of those files holds version 0 only.
Version 0 carries a dynamic label, a static label with tag-name-from, an important tag,
a dynamic tag, a long string, an unnamed tag, a template, and the dynamic payload tags.
Protocol also names every aggregate in ordering listed.

The wizard batch in console.gal runs next. The version count includes version 0,
so 10 versions means the wizard adds versions 1 through 9.
Each new version rewrites the Protocol ordering, and on Protocol and every aggregate
deletes payload_0, creates it again as marker { "back" }, and adds added_N.
create hash 0 closes version 0.

xslt/suffix.xsl appends the version index to every tag string.
It writes intermediate/suffix-fresh.xml.
The seal keeps the hashed version 0 text and writes those string changes into version 1.

Check actual_gal: protocol.gal, console.gal, and every Item file, with every version.
Version 0 stores the hash and the incoming strings. Version 1 holds the suffixed strings.
A later version holds payload_0 = marker { "back" } and added_N = { "new" }.
intermediate/galimatron.log shows each transform time, the run time, and the state size.
cpp2xml writes intermediate/lang-wizard.xml.
xml2cpp writes intermediate/lang-xslt.xml.
cpp2gal writes intermediate/lang-final.xml.
The post step writes intermediate/post-compiled.xml.

Abilities: aggregate files, dynamic label, static label, tag-name-from, important tag,
unnamed tag, template engine, ordering, wizard create version, wizard create ordering,
wizard delete tag, wizard create tag, wizard create hash, suffix.xsl, sealed version.

The console header is one line: the aggregate count, the version count, and the dynamic tag count.
The current source is 500 aggregates, 10 versions, 2 dynamic tags.

cmake --build Release --target benchmark


Full run
--------

The table is the measured dynamic run, saved in examples/benchmark/matrix/times.md.
Aggregates: 10, 20, 50, and 100. Versions: 10, 20, 50, and 100. Dynamic payload tags: 2 and 8.
Times are seconds from intermediate/galimatron.log.
xslt is Saxon at work: xml2saxon plus the user xslt. xml2cpp reads the result back.
seal is cpp2cpp_after_xslt. State is how much the process private memory grew during the run.

Processor: Intel Core i7-3770K @ 3.50 GHz, 4 cores, 8 threads.
RAM: 32 GB.
Compiler: Microsoft Visual C++ 19.51.36260.0 (Visual Studio 18 Build Tools, toolset 14.51.36231), x64.
Build: Release.
Optimizations: /O2 /Ob2 /DNDEBUG. Link: /INCREMENTAL:NO. Instruction set: SSE2.

agg   ver   tags   total   wizard   cpp2xml   xslt   xml2cpp   seal   cpp2gal   post xml   state
10    10    2      1.6     0.0      0.2       0.1    0.6       0.0    0.2       0.2        77.4 MB
10    10    8      3.1     0.0      0.4       0.2    1.2       0.1    0.4       0.4        97.4 MB
10    20    2      3.4     0.0      0.4       0.2    1.1       0.1    0.4       0.7        97.7 MB
10    20    8      6.3     0.0      0.9       0.5    2.6       0.2    0.8       0.9        107.5 MB
10    50    2      9.0     0.0      1.1       0.4    3.0       0.5    0.9       2.7        133.2 MB
10    50    8      16.6    0.1      2.4       1.1    6.4       0.7    2.0       3.3        191.9 MB
10    100   2      23.3    0.3      2.3       0.9    6.3       1.5    1.9       9.6        211.4 MB
10    100   8      39.7    0.6      4.9       2.1    13.6      2.2    4.1       11.7       292.4 MB
20    10    2      3.1     0.0      0.4       0.2    1.1       0.1    0.4       0.4        97.3 MB
20    10    8      5.9     0.0      0.9       0.4    2.4       0.2    0.7       0.6        115.5 MB
20    20    2      6.2     0.0      0.7       0.3    2.4       0.2    0.8       1.1        113.0 MB
20    20    8      11.6    0.0      1.7       0.7    4.9       0.4    1.6       1.6        174.0 MB
20    50    2      17.8    0.2      2.2       0.9    5.7       1.0    1.8       5.5        187.3 MB
20    50    8      32.0    0.4      4.6       1.9    12.6      1.7    3.8       6.3        275.3 MB
20    100   2      44.3    0.6      4.6       1.8    11.8      2.9    3.5       18.3       339.1 MB
20    100   8      75.7    1.3      9.9       4.0    26.8      4.4    7.6       21.0       486.7 MB
50    10    2      8.1     0.0      1.0       0.4    3.5       0.3    1.0       1.1        130.5 MB
50    10    8      14.6    0.0      2.1       1.0    6.0       0.7    2.0       1.8        191.3 MB
50    20    2      14.7    0.1      2.0       0.8    5.6       0.8    1.8       2.9        182.9 MB
50    20    8      29.5    0.2      4.5       1.8    12.7      1.3    3.8       4.3        260.8 MB
50    50    2      42.1    0.5      5.2       2.0    13.8      2.7    4.2       12.9       329.7 MB
50    50    8      77.1    0.9      11.8      4.5    29.1      3.9    10.2      15.7       507.6 MB
50    100   2      109.5   1.6      11.2      4.5    28.3      7.6    9.1       46.1       651.0 MB
50    100   8      184.4   3.6      24.3      10.6   63.3      11.2   19.2      50.6       1.0 GB
100   10    2      14.9    0.1      2.0       0.9    5.7       0.7    2.0       2.1        178.6 MB
100   10    8      28.4    0.1      4.3       1.9    12.0      1.2    3.9       3.5        262.7 MB
100   20    2      31.2    0.3      4.2       1.7    11.2      1.8    3.8       7.2        272.1 MB
100   20    8      56.9    0.5      9.0       3.5    23.7      2.8    7.7       8.4        429.4 MB
100   50    2      84.4    1.0      10.5      4.0    27.9      5.5    8.6       25.4       537.8 MB
100   50    8      157.0   2.0      23.0      9.1    59.6      8.6    18.3      34.7       941.6 MB
100   100   2      217.0   3.6      23.3      8.4    57.7      15.8   17.2      89.0       1.2 GB
100   100   8      365.3   6.7      49.2      21.1   119.9     27.3   36.7      101.6      1.9 GB

100 aggregates, 100 versions, 8 tags is 6 min 5 s, and the state grew by 1.9 GB.
xml2cpp and post xml are most of that time: 120 s and 102 s. Saxon with suffix.xsl takes 21 s.
State follows the versions, a little slower than O(V). At 10 versions, 100 aggregates stay near
179 MB with 2 tags and 263 MB with 8 tags.

cmake --build Release --target benchmark_matrix


Static tags
-----------

benchmark_matrix_static runs 12 combinations and writes examples/benchmark/matrix/times-static.md.
Aggregates: 10, 20, 50, and 500. Versions: 10. Static payload tags: 1, 50, and 100.
Those tags use a static label, so a new version does not copy them.
The same machine and the same Release build. State is the same private-memory growth.

agg   ver   tags   total   wizard   cpp2xml   xslt   xml2cpp   seal   cpp2gal   post xml   state
10    10    1      1.1     0.0      0.0       0.1    0.2       0.0    0.1       0.1        59.4 MB
10    10    50     3.3     0.0      0.3       0.2    0.8       0.1    0.4       1.0        92.0 MB
10    10    100    5.7     0.0      0.5       0.2    1.3       0.3    0.7       1.9        114.5 MB
20    10    1      1.8     0.0      0.2       0.1    0.5       0.0    0.2       0.3        81.1 MB
20    10    50     6.5     0.0      0.8       0.2    1.5       0.3    0.8       1.9        111.0 MB
20    10    100    10.8    0.0      0.9       0.4    2.6       0.5    1.5       3.6        149.0 MB
50    10    1      4.3     0.0      0.4       0.2    1.3       0.2    0.5       0.9        100.3 MB
50    10    50     15.6    0.0      1.5       0.6    3.8       0.8    2.4       4.9        186.8 MB
50    10    100    26.7    0.0      2.4       0.9    6.4       1.4    4.0       8.8        258.6 MB
500   10    1      48.3    0.4      5.8       2.0    14.2      5.2    5.8       8.9        352.0 MB
500   10    50     159.3   0.5      15.4      5.3    38.8      11.6   22.6      49.0       986.6 MB
500   10    100    273.6   0.9      25.7      9.0    64.1      19.4   39.5      89.3       1.7 GB

500 aggregates, 10 versions, 100 static tags is 4 min 34 s, and the state grew by 1.7 GB.
post xml, xml2cpp, and cpp2gal are most of that time: 89 s, 64 s, and 40 s.
The twelve runs together are 9 min 17 s of Galimatron time.

Versions stay at 10. A new version does not copy the static tags, so time is about O(A T^{0.4}).
From 1 static tag to 100, the run grows about 5 to 6 times at every aggregate count.
T^{0.4} grows 6.3 times over that step.
Fifty times the aggregates (10 to 500) multiplies a 1-tag run by about 44,
a 50-tag run by about 48, and a 100-tag run by about 48.
O(A) grows 50 times over that step.
At 500 aggregates the wizard stays under a second, O(1) in the tag count.
State grows with both axes, more slowly than the time.
One static tag at 500 aggregates is 352 MB.
One hundred static tags there is 1.7 GB.

cmake --build Release --target benchmark_matrix_static


Static tags, no xslt
---------------------

benchmark_matrix_static_no_xslt uses gal-static-no-xslt.cfg. That file has no user xslt,
so cpp2xml, suffix.xsl, xml2cpp, and the seal are not in the run, and Saxon is not created.
The same static grid. The log has no "xslt processor" line.
Times are examples/benchmark/matrix/times-static-no-xslt.md.

agg   ver   tags   total   wizard   cpp2gal   post xml   state
10    10    1      0.5     0.0      0.1       0.2        5.4 MB
10    10    50     1.7     0.0      0.3       1.0        18.8 MB
10    10    100    3.0     0.0      0.5       1.9        29.9 MB
20    10    1      0.7     0.0      0.2       0.3        10.3 MB
20    10    50     3.3     0.0      0.6       2.1        31.3 MB
20    10    100    5.8     0.0      1.0       3.8        55.1 MB
50    10    1      1.8     0.0      0.5       0.9        21.4 MB
50    10    50     8.2     0.0      1.5       5.1        69.7 MB
50    10    100    15.3    0.0      2.5       10.3       125.4 MB
500   10    1      20.4    0.4      7.0       9.1        185.0 MB
500   10    50     81.4    0.5      16.6      50.4       681.9 MB
500   10    100    145.6   0.7      27.0      93.1       1.1 GB

500 aggregates, 10 versions, 100 static tags is 2 min 26 s, down from 4 min 34 s with suffix.xsl.
The state grew by 1.1 GB, down from 1.7 GB.
post xml is the slow step there: 93 s. The wizard stays under a second.
The twelve runs together are 4 min 48 s of Galimatron time, down from 9 min 17 s.

cmake --build Release --target benchmark_matrix_static_no_xslt


Complexity
----------

A is the aggregate count, V the version count, T the payload-tag count.
The orders are the measured grids.

Dynamic tags. Total time is about O(A V^{1.1} T^{0.4}).
Versions dominate, about O(V^{1.1}).
Ten times the versions (10 to 100) multiplies the run by about 13 to 15.
V^{1.1} multiplies that step by about 13.
Five times the versions (10 to 50) multiplies the run by about 5 to 6.
V^{1.1} multiplies that step by about 6.
The copied dynamic tags grow with every version.
Tags are about O(T^{0.4}). Four times the tags (2 to 8) multiplies the run by about 1.7 to 1.9.
Aggregates are about O(A). Twice the aggregates multiplies the run by about 2.

Static tags keep V fixed at 10, so the version factor drops out.
Total time is about O(A T^{0.4}), and the wizard is O(1) in T.
