#!/bin/bash

cd ../build

./bin/aff3ct            \
    --simu-type   BFER  \
    --code-type   POLAR \
    -K            2048  \
    -N            4096  \
    --snr-min     2.0   \
    --snr-max     3.01  \
    --snr-step    0.1   \
    --max-fe      100   \
    --dec-algo    SC    \
    --dec-implem  FAST  \
    --domain      LLR   \
    $1 $2 $3 $4 $5 $6 $7 $8 $9 $10