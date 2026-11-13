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
Aggregates: 50 and 100. Versions: 10, 50, and 100. Dynamic payload tags: 2 and 8.
Times are seconds from intermediate/galimatron.log.
State is how much the process private memory grew during the run.

Processor: Intel Core i7-3770K @ 3.50 GHz, 4 cores, 8 threads.
RAM: 32 GB.
Compiler: Microsoft Visual C++ 19.51.36260.0 (Visual Studio 18 Build Tools, toolset 14.51.36231), x64.
Build: Release.
Optimizations: /O2 /Ob2 /DNDEBUG. Link: /INCREMENTAL:NO. Instruction set: SSE2.

agg   ver   tags   total     wizard   cpp2xml   xslt     xml2cpp   cpp2gal   post xml   state
50    10    2      6.9       0.2      1.0       1.2      1.4       1.0       1.0       152.0 MB
50    10    8      14.1      0.2      2.0       3.4      3.6       2.0       1.6       201.0 MB
50    50    2      50.4      3.5      4.9       11.3     12.9      4.2       11.1      446.5 MB
50    50    8      128.5     3.5      10.0      42.7     45.7      9.1       13.8      798.7 MB
50    100   2      151.8     11.2     10.2      36.9     41.2      8.3       37.5      1.1 GB
50    100   8      419.4     12.9     22.5      152.8    160.4     18.3      43.3      1.6 GB
100   10    2      14.8      0.8      1.9       3.0      3.5       1.9       2.1       261.8 MB
100   10    8      32.7      0.8      3.8       8.9      10.0      4.0       3.1       276.9 MB
100   50    2      134.1     12.3     9.5       37.0     39.9      8.5       21.5      808.2 MB
100   50    8      393.3     12.6     20.3      150.9    157.1     17.6      27.1      1.3 GB
100   100   2      436.4     45.0     20.2      130.6    137.6     16.2      73.4      2.1 GB
100   100   8      1383.9    47.4     43.5      568.0    580.3     35.2      88.8      2.9 GB

100 aggregates, 100 versions, 8 tags is 23 min 4 s, and the state grew by 2.9 GB.
suffix.xsl and lang-xslt.xml are most of that time: 568 s and 580 s.
State follows the versions, about O(V). At 10 versions, 100 aggregates stay near 260 MB for both tag counts.

cmake --build Release --target benchmark_matrix

The target now also lists aggregates 10 and 20 and versions 20. That wider grid has not been run.


Static tags
-----------

benchmark_matrix_static runs 12 combinations and writes examples/benchmark/matrix/times-static.md.
Aggregates: 10, 20, 50, and 500. Versions: 10. Static payload tags: 1, 50, and 100.
Those tags use a static label, so a new version does not copy them.
The same machine and the same Release build. State is the same private-memory growth.

agg   ver   tags   total     wizard   cpp2xml   xslt     xml2cpp   cpp2gal   post xml   state
10    10    1      0.8       0.0      0.1       0.1      0.1       0.1       0.1       73.4 MB
10    10    50     2.9       0.0      0.2       0.3      0.3       0.4       0.9       111.3 MB
10    10    100    4.9       0.0      0.5       0.5      0.6       0.8       1.5       135.2 MB
20    10    1      1.5       0.0      0.1       0.2      0.2       0.2       0.3       87.9 MB
20    10    50     5.1       0.0      0.5       0.6      0.7       0.8       1.6       154.4 MB
20    10    100    9.5       0.0      0.8       1.0      1.3       1.5       3.2       204.2 MB
50    10    1      3.9       0.2      0.4       0.5      0.6       0.5       0.8       114.8 MB
50    10    50     14.5      0.2      1.4       1.7      2.2       2.2       4.2       298.6 MB
50    10    100    26.2      0.2      2.2       3.7      4.4       3.8       7.8       430.8 MB
500   10    1      76.6      22.0     5.6       11.2     13.6      6.0       7.8       444.3 MB
500   10    50     271.0     24.0     14.2      70.9     75.1      21.2      41.1      1.6 GB
500   10    100    584.9     21.3     23.5      187.1    195.5     38.1      79.3      2.9 GB

500 aggregates, 10 versions, 100 static tags is 9 min 45 s, and the state grew by 2.9 GB.
suffix.xsl and lang-xslt.xml are most of that time: 187 s and 196 s.
The twelve runs together are 16 min 42 s of Galimatron time.

Versions stay at 10. A new version does not copy the static tags, so time is about O(A T^{1/2}).
From 1 static tag to 100, the run grows about 6 times at 10 aggregates and about 8 times at 500.
T^{1/2} grows 10 times over that step.
Fifty times the aggregates (10 to 500) multiplies a 1-tag run by about 96,
a 50-tag run by about 53, and a 100-tag run by about 119.
O(A) grows 50 times over that step.
At 500 aggregates the wizard stays near 22 s, O(1) in the tag count.
The added tags land in suffix.xsl and lang-xslt.xml.
State grows with both axes, more slowly than the time.
One static tag at 500 aggregates is 444 MB.
One hundred static tags there is 2.9 GB, the same growth as 100 aggregates, 100 versions, and 8 dynamic tags.

cmake --build Release --target benchmark_matrix_static


Static tags, no xslt
---------------------

benchmark_matrix_static_no_xslt uses gal-static-no-xslt.cfg. That file has no user xslt,
so cpp2xml, suffix.xsl, and xml2cpp are not in the run, and Saxon is not created.
The same static grid. The log has no "xslt processor" line.
Times are examples/benchmark/matrix/times-static-no-xslt.md.

agg   ver   tags   total     wizard   cpp2gal   post xml   state
10    10    1      0.4       0.0      0.1       0.2       5.1 MB
10    10    50     1.8       0.0      0.3       1.1       17.8 MB
10    10    100    3.1       0.0      0.5       2.0       30.3 MB
20    10    1      0.7       0.0      0.2       0.3       10.1 MB
20    10    50     3.3       0.0      0.6       2.0       31.5 MB
20    10    100    5.9       0.0      1.0       3.8       56.4 MB
50    10    1      2.2       0.2      0.5       0.9       21.7 MB
50    10    50     7.9       0.2      1.4       4.9       70.2 MB
50    10    100    14.7      0.2      2.6       9.3       126.3 MB
500   10    1      44.5      23.4     6.7       8.9       185.0 MB
500   10    50     98.5      20.7     15.9      47.4      689.2 MB
500   10    100    156.6     21.0     24.8      86.4      1.1 GB

500 aggregates, 10 versions, 100 static tags is 2 min 37 s, down from 9 min 45 s with suffix.xsl.
The state grew by 1.1 GB, down from 2.9 GB.
post xml is the slow step there: 86 s. The wizard stays near 21 s.
The twelve runs together are 5 min 40 s of Galimatron time, down from 16 min 42 s.

cmake --build Release --target benchmark_matrix_static_no_xslt


Complexity
----------

A is the aggregate count, V the version count, T the payload-tag count.
The orders are the measured grids.

Dynamic tags. Total time is about O(A V^{3/2} T^{1/2}).
Versions dominate, about O(V^{3/2}).
Ten times the versions (10 to 100) multiplies the run by about 22 to 42.
V^{3/2} multiplies that step by about 32.
Five times the versions (10 to 50) multiplies the run by about 7 to 12.
V^{3/2} multiplies that step by about 11.
The copied dynamic tags and the suffix.xsl and lang-xslt.xml walks grow with every version.
Tags are about O(T^{1/2}) on a 10-version run.
Four times the tags (2 to 8) multiplies that run by about 2.
On a 100-version run the same four times multiplies it by about 3, toward O(T).
Aggregates are about O(A) to O(A^{3/2}). Twice the aggregates multiplies the run by about 2 to 3.

Static tags keep V fixed at 10, so the version factor drops out.
Total time is about O(A T^{1/2}), and the wizard is O(1) in T.
