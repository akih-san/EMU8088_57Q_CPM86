# EMU8088_57Q<br>
<br>
PIC18F47Q43はDIP40ピンで、電脳伝説さんがそれを使用したSBCであるEMUZ80を<br>
公開されたのがきっかけで、その後コアな愛好者によって、色々な拡張や<br>
新しいSBCが公開されています。<br>
<br>
PIC18F57Q43は、QFP48ピンのパッケージに収められており、仕様は47Qと変わりませんが、<br>
I/Oが8ピン多いのが特徴です。<br>
<br>
今回は、このPIC18F57Q43を使った8088/用のSBCを作成しました。<br>
V20はD70108D-8が動作します。<br>
メモリはAS6C4008(512kw x 8bit)を2個使用し、8088のフル空間（1MB）上で、<br>
CP/M-86が動作します。（将来的には、MS-DOSを動かしたい）<br>
<br>
EMU8088_57Q SBCボード

![EMU8088 1](photo/8088_RAM_R.1.5.png)

写真は、Rev1.5
![EMU8088 2](photo/P1020471.JPG)


Rev1.4とRev1.5(左側がRev1.4）
![EMU8088 2](photo/P1020472.JPG)


CP/M-86起動画面<br>
![EMU8088 1](photo/p224950.png)


# 動作確認したCPU
![EMU8088 1](photo/P1020476.JPG)

V20は、D70108D-8での動作が確認出来ている。<br>
残念ながら、D70108C-8, D70108C-10では、動作が不安定でほとんど<br>
起動出来ていない。READY信号のタイミングはUPD70108のタイミングを<br>
満たすようにCLCの設計を詰めたつもりである。<br>
IOアクセスとメモリアクセスが干渉してると思われるが、起動時の<br>
ワークエリアアドレスを変えたりすると起動出来たりする。<br>
原因は掴めていない。<br>
<br>
まぁ、このSBCは、Z80エミュレーションをすることは、目的ではないので、<br>
原因の追及は別の機会に持ち越しで、ペンディングとしたい。<br>
誰か分かる方がいらっしゃったら、連絡いただけるとありがたいです。<br>
<br>
試したV20（安定して動作したのはD70108D-8）
![EMU8088 V20](photo/GESYJGmaoAAoq6Z.jpg)

# ファームウェア
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
