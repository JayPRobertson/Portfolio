# Additional Test Cases for A2

It is recommended that you use the small test case found in the
assignment specification until success. The test cases provided 
here may run significantly longer than desired -- or even than your
code will be tested on.

Nevertheless, here are is a summary of input-output combinations to
support development.

## Table of Content:

- [Small Test Case](#Small-Test)
    - [Small Input](#inputtxt)
    - [Small Output](#outputtxt) 

- [Medium Test Case](#Medium-Test)
    - [Medium Input](#inputtxt-1) 
    - [Medium Output](#outputtxt-1)

- [Starvation Test Case](#Starvation-Test)
    - [Starvation Input](#inputtxt-2) 
    - [Starvation Output](#outputtxt-2) 

## Small-Test

- [Small Input](#inputtxt)
- [Small Output](#outputtxt) 

### input.txt:

```
e 10 6
W 6 7
E 3 10
```

### output.txt:

```
00:00:00.3 Train  2 is ready to go East
00:00:00.3 Train  2 is ON the main track going East
00:00:00.6 Train  1 is ready to go West
00:00:01.0 Train  0 is ready to go East
00:00:01.3 Train  2 is OFF the main track after going East
00:00:01.3 Train  1 is ON the main track going West
00:00:02.0 Train  1 is OFF the main track after going West
00:00:02.0 Train  0 is ON the main track going East
00:00:02.6 Train  0 is OFF the main track after going East
```

## Medium-Test

- [Medium Input](#inputtxt-1) 
- [Medium Output](#outputtxt-1)

### input.txt:

```
E 58 73
w 40 4
W 34 41
E 76 9
W 84 29
e 77 57
w 13 22
E 60 35
E 39 15
W 98 23
```

### output.txt:

```
00:00:01.3 Train  6 is ready to go West
00:00:01.3 Train  6 is ON the main track going West
00:00:03.4 Train  2 is ready to go West
00:00:03.5 Train  6 is OFF the main track after going West
00:00:03.5 Train  2 is ON the main track going West
00:00:03.9 Train  8 is ready to go East
00:00:04.0 Train  1 is ready to go West
00:00:05.8 Train  0 is ready to go East
00:00:06.0 Train  7 is ready to go East
00:00:07.6 Train  3 is ready to go East
00:00:07.6 Train  2 is OFF the main track after going West
00:00:07.6 Train  8 is ON the main track going East
00:00:07.7 Train  5 is ready to go East
00:00:08.4 Train  4 is ready to go West
00:00:09.1 Train  8 is OFF the main track after going East
00:00:09.1 Train  4 is ON the main track going West
00:00:09.8 Train  9 is ready to go West
00:00:12.0 Train  4 is OFF the main track after going West
00:00:12.0 Train  0 is ON the main track going East
00:00:19.3 Train  0 is OFF the main track after going East
00:00:19.3 Train  9 is ON the main track going West
00:00:21.6 Train  9 is OFF the main track after going West
00:00:21.6 Train  7 is ON the main track going East
00:00:25.1 Train  7 is OFF the main track after going East
00:00:25.2 Train  3 is ON the main track going East
00:00:26.1 Train  3 is OFF the main track after going East
00:00:26.1 Train  1 is ON the main track going West
00:00:26.5 Train  1 is OFF the main track after going West
00:00:26.5 Train  5 is ON the main track going East
00:00:32.2 Train  5 is OFF the main track after going East
```

## Starvation-Test 

- [Starvation Input](#inputtxt-2) 
- [Starvation Output](#outputtxt-2) 

### input.txt:

```
e 5 1
w 1 6
W 2 1
W 3 1
```

### output.txt:
```
00:00:00.1 Train  1 is ready to go West
00:00:00.1 Train  1 is ON the main track going West
00:00:00.2 Train  2 is ready to go West
00:00:00.3 Train  3 is ready to go West
00:00:00.5 Train  0 is ready to go East
00:00:00.7 Train  1 is OFF the main track after going West
00:00:00.7 Train  2 is ON the main track going West
00:00:00.8 Train  2 is OFF the main track after going West
00:00:00.8 Train  0 is ON the main track going East
00:00:00.9 Train  0 is OFF the main track after going East
00:00:00.9 Train  3 is ON the main track going West
00:00:01.0 Train  3 is OFF the main track after going West

```