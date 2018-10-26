# MTND
This is an implementation of a non deterministic Turing machine, in the single tape variant and only acceptors.
Input file structure: first the transition function is provided,
hence the acceptance states and a maximum limit on the number of steps for a single computation (to avoid the problem of machines that do not end), finally a series of strings to be read by the machine.
In output we expect a file containing 0 for the strings that are not accepted and 1
for those accepted;  due to the limit on the number of steps, the
result can also be U if we haven't accepted yet.

Input file example: language = ww

tr
0 a c R 1\n
0 b c R 2
1 a a R 1
1 a d L 3
1 b b R 1
2 a a R 2
2 b b R 2
2 b d L 3
3 a a L 3
3 b b L 3
3 c c R 4
4 d d R 10
4 a c R 5
4 b c R 6
5 a a R 5
5 b b R 5
5 d d R 7
6 a a R 6
6 b b R 6
6 d d R 8
7 d d R 7
7 a d L 9
8 d d R 8
8 b d L 9
9 d d L 9
9 a a L 3
9 b b L 3
9 c c R 10
10 d d R 10
10 _ _ S 11
acc
11
max
2000
run
aabaab
bbabbb
ababa
babaaababaaa
ababaabababbabbbbbabbbbabaaababaaaababaabababbabbbbbabbbbabaaababaaa

standard output:
1
0
0
1
U

p.s: transition 4 b c R 6 --> means that if i am in state 4 and i am reading character b from the tape, i have to write c,i have to go in state 6 and my head has to go right(R).
