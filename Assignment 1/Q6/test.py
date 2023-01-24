import math

# Function to print the distinct prime factors of a number
def prime_factors(n):
    # Store the distinct prime factors
    factors = []
    # 2 is the only even prime number
    while n % 2 == 0:
        factors.append(2)
        n = n / 2
    # n must be odd at this point
    for i in range(3, int(math.sqrt(n))+1, 2):
        # while i divides n , print i and divide n by i
        while n % i == 0:
            factors.append(i)
            n = n / i
    # Condition if n is a prime
    # number greater than 2
    if n > 2:
        factors.append(n)
    # remove duplicate values
    factors = list(set(factors))
    return factors

# Test the function
print(prime_factors(100)) # Output: [2, 5]

with open("myfile.txt") as fp:
    Lines = fp.readlines()
    for line in Lines:
        print("Line{}: {}".format(count, line.strip()))