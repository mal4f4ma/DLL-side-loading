f = open('functions.txt', 'r')
dest = open('final.txt', 'w')
lines = f.read().splitlines()
expor = 'BOOL __cdecl {}() {{ return 1;}}'
for line in lines:
	print(line)
	dest.write('extern "C" __declspec(dllexport)\n' + expor.format(line) + '\n')
dest.close()
