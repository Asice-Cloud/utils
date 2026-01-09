print("Calling C function 'add' via dynref:")
local r = dynref.call("add", 11, 31)
print(" add(11,31) =>", r)

print("Calling C function 'greet' via dynref:")
local s = dynref.call("greet", "Alice")
print(" greet('Alice') =>", s)
