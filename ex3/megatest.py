return_code = []
i = 0

for x in range(4000):
	return_code[x] = subprocess.call("./RUNME", shell=True)
	if return_code[x] is not 0:
		i = i + 1

print(i)

