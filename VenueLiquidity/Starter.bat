set directory="./x64/Debug"
set h5File="./VenueLiquidity/Resources/ticks.20140109.h5"
cd %directory% 

start %directory% VenueLiquidity.exe 0 1000 %h5File%
start %directory% VenueLiquidity.exe 1000 2000 %h5File%
start %directory% VenueLiquidity.exe 2000 3000 %h5File%
start %directory% VenueLiquidity.exe 3000 4000 %h5File%
start %directory% VenueLiquidity.exe 4000 5000 %h5File%
start %directory% VenueLiquidity.exe 5000 6000 %h5File%
start %directory% VenueLiquidity.exe 6000 7000 %h5File%
start %directory% VenueLiquidity.exe 7000 7930 %h5File%



