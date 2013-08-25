#!/bin/bash

countfailed=0
counttotal=0

for i in *.result; do
  ((counttotal += 1))
  outname=$(echo $i | sed -e 's/.result$/.out/g')
  diff $i $outname > /dev/null
  if [ $? -ne 0 ]; then
    echo "Test $(echo $i | sed -e 's/.result$//g') fails!"
    echo "DIFF between actual (left) and expected (right) follows :"
    diff $i $outname
    echo "END OF DIFF"
    ((countfailed += 1))
  fi
done

if [ ${countfailed} -eq 0 ]; then
  echo "All ${counttotal} tests pass!"
else
  echo "${countfailed}/${counttotal} tests failed!"
fi
