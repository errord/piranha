import pyranha

# Various ways of constructing an int.
print(42)
print(int("42"))
print(int(42.123))

# Arbitrarily large ints are supported.
print(12345678987654321234567898765432)

# Interoperability with float.
print(43. - 1)
# Truncated division in Python 2.x, floating-point division in Python 3.x.
print(85 / 2)

# Exponentiation via the '**' operator.
print(42**2)