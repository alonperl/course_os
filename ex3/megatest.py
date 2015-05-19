import subprocess

a = 5000
i = 0

for x in range(a):
	return_code = subprocess.call("./RUNME", shell=True)
	if return_code is not 0:
		i = i + 1

print("runned ", a, ", failed ", i)

