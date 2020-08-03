from test_framework.heavyhash_imp import getPoWHash, default_matrix
from codecs import encode

""" Compares hashes produced by the heavy hash algorithm to the hardcoded values """

class HeavyHashTest():

    def __init__(self):
        self.inputs  = [b'\xC1\xEC\xFD\xFC',
                        b'\x4F\x1A\xA2\x5D',
                        b'\xb9']

        self.samples = ["39387f2e64e7c08d3ce0da8c491b4fcf2c862798dedb4690d819de7926aa4ecb",
                        "10cae3725f9c15423af15d6438b93a1c832b97a9a1844a6938c299834390851b",
                        "e95d6452c135175a7c2adf61ff039956669ae0e62018d0baf48a1daa1cf5ad66"]

    def run_test(self, matrix):
        i = 0

        for sample in self.samples:
            digest = encode(getPoWHash(matrix, self.inputs[i]), 'hex_codec').decode('ascii')

            assert digest == sample, "Heavy hash digest is wrong!"
            assert getPoWHash(matrix, self.inputs[i]) == bytes.fromhex(sample)
            i += 1


if __name__ == '__main__':
    heavy_hash_test = HeavyHashTest()
    heavy_hash_test.run_test(default_matrix)