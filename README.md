# EMU8088/V20_57Q<br>
<br>
PIC18F47Q43はDIP40ピンで、電脳伝説さんがそれを使用したSBCであるEMUZ80を<br>
公開されたのがきっかけで、その後コアな愛好者によって、色々な拡張や<br>
新しいSBCが公開されています。<br>
<br>
PIC18F57Q43は、QFP48ピンのパッケージに収められており、仕様は47Qと変わりませんが、<br>
I/Oが8ピン多いのが特徴です。<br>
<br>
今回は、このPIC18F57Q43を使った8088/V20用のSBCを作成しました。<br>
メモリはAS6C4008(512kw x 8bit)を2個使用し、8088/V20のフル空間<br>
（1MB）上で、CP/M-86が動作します。（将来的には、MS-DOSを動かしたい）<br>
<br>
EMU8088/V20_57Q Rev1.5 SBCボード<br>

![EMU8088 1](photo/P1020470.JPG)
<br>
EMU8088/V20_57Q Rev1.5 SBCボード<br>


@hanyazouさんが作成したZ80で動作しているCP/M-80のFWをベースに<br>
EMU8088/V20_57Q0用のFWとして動作するように修正を加え、CP/M-86を<br>
インプリメントしました。<br>
<br>
FWのI/Oモジュールは、ほぼ新設計となります。<br>
DISK I/OとFatFs、及びSPIについては、ほぼ未修整で使用しています。<br>
<br>
FWのソースのコンパイルは、マイクロチップ社の<br>
<br>
「MPLAB® X Integrated Development Environment (IDE)」<br>
<br>
を使っています。（MPLAB X IDE v6.10）コンパイラは、XC8を使用しています。<br>
https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide<br>
<br>
8088/V20用のアセンブラは、Macro Assembler AS V1.42を使用しています。<br>
http://john.ccac.rwth-aachen.de:8000/as/<br>
<br>
FatFsはR0.15を使用しています。<br>
＜FatFs - Generic FAT Filesystem Module＞<br>
http://elm-chan.org/fsw/ff/00index_e.html<br>
<br>
SDカード上のCP/Mイメージファイルの作成は、CpmtoolsGUIを利用しています。<br>
＜CpmtoolsGUI - neko Java Home Page＞<br>
http://star.gmobb.jp/koji/cgi/wiki.cgi?page=CpmtoolsGUI<br>
<br>
<br>
＜＠hanyazouさんのソース＞<br>
https://github.com/hanyazou/SuperMEZ80/tree/mez80ram-cpm<br>
<br>
<br>
＜@electrelicさんのユニバーサルモニタ＞<br>
https://electrelic.com/electrelic/node/1317<br>
<br>
<br>
＜参考＞<br>
・EMUZ80<br>
EUMZ80はZ80CPUとPIC18F47Q43のDIP40ピンIC2つで構成されるシンプルなコンピュータです。<br>
<br>
＜電脳伝説 - EMUZ80が完成＞  <br>
https://vintagechips.wordpress.com/2022/03/05/emuz80_reference  <br>
＜EMUZ80専用プリント基板 - オレンジピコショップ＞  <br>
https://store.shopping.yahoo.co.jp/orangepicoshop/pico-a-051.html<br>
<br>
・SuperMEZ80<br>
SuperMEZ80は、EMUZ80にSRAMを追加し、Z80をノーウェイトで動かすことができるメザニンボードです<br>
<br>
SuperMEZ80<br>
https://github.com/satoshiokue/SuperMEZ80<br>
<br>
