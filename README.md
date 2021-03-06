![ef](./ef.png "EF (Efficient Functional)")

### EF (Efficient [Functional](https://en.wikipedia.org/wiki/Functional_programming)) or (Extended [FaLSE](https://esolangs.org/wiki/FALSE) ?) scripting language

Some links for ***FALSE*** [#](http://wiki.c2.com/?FalseLanguage) [##](http://strlen.com/false-language/)  [###](https://ru.wikipedia.org/wiki/FALSE) 

### About EF
- easy to learned :grin:
- replasment of [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) [Perl](https://en.wikipedia.org/wiki/Perl) [Erlang](https://en.wikipedia.org/wiki/Erlang_(programming_language)) ... :blush:
- designing an as ***powerful language*** as possible with a tiny implementation
- [stl](https://en.wikipedia.org/wiki/Standard_Template_Library) only 
- stack based [Reverse Polish notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation)
- ***threads*** support
- ***timers***
- **os** functions
- **sql** functions
- **ip** functions
- **ef** based on [uv](https://github.com/libuv/libuv)
- easy make addons - ***plugins***

***Binary Releases for Debian*** [bin](./bin)

Projects files on [Qt Creator](https://www.qt.io/)

#### Hallo
```
(aa bb bb)mq
20 N ~(v0 '%.f HALLO!'P l)
20 ~(v0 '%.f HALLO!'P l)
#[(?? ) 100]TM
#PID $ l KILL
E
```

#### EF syntax [sourse](./ef_proj/ef_lang/ef_lang.cpp "ef_lang.cpp"):

`lg` log **Globals**  
`ls` log **Stack**  
`l` print **Top Stack** var  
`#` **comment** to end of string  
`##  ... ##` strings **comment**  
`,:\n\r\s` **spacers**  
`"some_characters..."` `'some_characters...'` **string**  
`[..]` **array**  
```
['one','two',99,144]   
```

`{k1:v1,k2:v2; k3 v3 ...}` **map**  
```
{'a' 12312,'b'[1 3 5 76 T F N 'разработка интересного языка'] 
'66'555 'm' [{} 44 55 []] '111'1 '8' 'fgsdgsd'} #ls
``` 

`(..)` **functional literal**  (after this stack stay the same!)
```
(?? RD l % 0 SL RP)
```
`some_not_used_string` **Atom** (Global Key for some...)  
**Known Globals:** `A` ARGV `T` True `F` False `N` Nil `R` Return `B` Break `RP` Repeate  
**Known Functions:**
- `er` `l` `lT` `lF` `ls` `lg` `TS` (to string) `FS` (from string) `SP` (split) `JN`(join) `P` (printf-like) `L` (length) `C` (clear)
- `?` if `~` iterator (all are not easy)
- `Q M S D` make deQue , Map , String , Double ( C+CMD Constant...)
- `$(...)` dup `%` drop `%<` drop all to initial function call `\` swap `@(...)` to top from begin `v(...)` to top from calling tree `.(...)` Sets! 

- `+` plus... `-` minus `*` mul... `/` div `//` % `^` named function to stack `++` `--` `SR` sort `RS` revers sort
-  `& | ! > < >= <= != = =N !N`
-  `??` must Quit?->return `E` exit `SL` sleep
- `TM` timer
  ```
  [(?? N RD $ N != ?(ls i.f5) ) 100]TM
  ```
- `TH` thread
  ```
  [(?? RD l % 0 SL RP) bc]TH
  ```
- `LC` make Locker `RL WL RU WU`
- **OS functions** `RM REN MKD DIR CDIR PID KILL RD RDB WR`
- **SQL** `DB SQL EX !! NX >> PR ER RW`
- **ip** ...
- `INC` **include script** `LD` load **Plugin**
- **regExp PCRE** `RE REQ`
- **Spawn** `sp.N sp.A sp.K`

#### Plugins

[leveldb core](./ef_proj/ef_lev/ef_lev.cpp "ef_lev.cpp") **->** [leveldb example](./bin/lev.ef "lev.ef")

#### Examples

[Only Look and see](./tst.ef "tst.ef") 


[***License***](https://www.gnu.org/licenses/gpl.html "GPL")

[mail](mailto:hserg1965@rambler.ru "Send...")
