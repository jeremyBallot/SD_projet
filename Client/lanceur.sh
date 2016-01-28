#!/bin/bash
J=0;
while [ $J -lt 16 ]
do
{
   ./client 127.0.0.1 &
   let J++;
}
done
echo "J = "$J;


