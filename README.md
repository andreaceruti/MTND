# MTND
This is an implementation of a non deterministic Turing machine, in the single tape variant and only acceptors.
Input file structure: first the transition function is provided,
hence the acceptance states and a maximum limit on the number of steps for a single computation (to avoid the problem of machines that do not end), finally a series of strings to be read by the machine.
In output we expect a file containing 0 for the strings that are not accepted and 1
for those accepted;  due to the limit on the number of steps, the
result can also be U if we haven't accepted yet.

