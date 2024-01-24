Get-Content cbios_*.bin -Encoding Byte | Set-Content cbios.bin -Encoding Byte
.\mkinc.exe .\cbios.bin | sc cbios.inc
