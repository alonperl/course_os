import subprocess

a = 1
i = 0

for x in range(a):
	return_code = subprocess.call("./RUNME", stdout=subprocess.PIPE)
	if return_code is not 0:
		i = i + 1

print("runned ", a, ", failed ", i)

