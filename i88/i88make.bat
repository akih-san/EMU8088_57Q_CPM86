asw -L unimon_8086.asm
p2bin unimon_8086.p
powershell ./unimon_8086.ps1
asw -L cbios.asm
p2bin cbios.p cbios_code.bin -segment code
p2bin cbios.p cbios_data.bin -segment data
powershell ./cbios.ps1

