Get-Content cbios_*.bin -Encoding Byte | Set-Content cbios.bin -Encoding Byte
.\bin2inc.exe .\cbios.bin | sc cbios.inc
