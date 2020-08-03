from test_framework.rank import rankMatrix

""" Complementary functions for matrix generation and seeding """
def generate_heavyhash_matrix(s):
    matrix = []
    check_matrix = []

    for i in range(64):
        matrix.append([])
        check_matrix.append([])
        for j in range(64):
            matrix[i].append(0)
            check_matrix[i].append(0)

    for i in range(0, 64):
        for j in range(0, 64, 16):
            value, s = xoshiro256pp(s)
            for shift in range(0, 16):
                matrix[i][j + shift] = (value >> (60-4 * shift)) & 0xF
                check_matrix[i][j + shift] = matrix[i][j + shift]

    rank = rankMatrix(check_matrix).rankOfMatrix(check_matrix)
    if (rank != 64):
        generate_heavyhash_matrix(s)

    return matrix

def get_uint64(data, pos):
    ptr = pos*8
    uint64 = data[ptr] | (data[ptr+1] << 8) | (data[ptr+2] << 16) | (data[ptr+3] << 24) | (data[ptr+4] << 32) |\
             (data[ptr+5] << 40) | (data[ptr+6] << 48) | (data[ptr+7] << 56)

    return uint64

""" XOSHIRO256++ ALGORITHM
    A pseudo random number generator used in Heavy Hashing for 
    matrix generation """
def rol64(x, k):
    return ( (x << k) & 0xFFFFFFFFFFFFFFFF ) | ( (x >> (64 - k)) & 0xFFFFFFFFFFFFFFFF )

def xoshiro256pp_seeding(seed):
    s = [0, 0, 0, 0]
    for i in range(4):
        s[i] = get_uint64(seed, i)
    return s

def xoshiro256pp(s):
    result = (rol64((s[0]+ s[3]) & 0xFFFFFFFFFFFFFFFF, 23) + s[0]) & 0xFFFFFFFFFFFFFFFF
    t = (s[1] << 17) & 0xFFFFFFFFFFFFFFFF

    s[2] ^= s[0]
    s[3] ^= s[1]
    s[1] ^= s[2]
    s[0] ^= s[3]

    s[2] ^= t
    s[3] = rol64(s[3] & 0xFFFFFFFFFFFFFFFF, 45)

    return result, s

