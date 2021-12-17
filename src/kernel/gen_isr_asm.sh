#!/usr/bin/env zsh

test -f isr.S && rm isr.S
cp isr.S.base isr.S

for i ({0..255}); do 
  cat  >> isr.S << EOF
isr$i:
  push ebp
  mov ebp, esp
  push $i
  jmp catchall
EOF
done

x=""
for i ({0..255}); do 
  if [ $i = 255 ]; then 
    x="$x isr$i"
  else
    x="$x isr$i,"
  fi
done

echo "isrs dd $x" >> isr.S