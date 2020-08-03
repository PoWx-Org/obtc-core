class rankMatrix():
    def __init__(self, matrix):
        self.rows = len(matrix)
        self.cols = len(matrix[0])

    def swap(self, matrix, row1, row2, col):
        for i in range(col):
            temp = matrix[row1][i]
            matrix[row1][i] = matrix[row2][i]
            matrix[row2][i] = temp

    def rankOfMatrix(self, matrix):
        rank = self.cols
        for row in range(0, rank, 1):

            if matrix[row][row] != 0:
                for col in range(0, self.rows, 1):
                    if col != row:

                        multiplier = (matrix[col][row] /
                                      matrix[row][row])
                        for i in range(rank):
                            matrix[col][i] -= (multiplier *
                                               matrix[row][i])
            else:
                reduce = True
                for i in range(row + 1, self.rows, 1):

                    if matrix[i][row] != 0:
                        self.swap(matrix, row, i, rank)
                        reduce = False
                        break
                if reduce:
                    rank -= 1
                    for i in range(0, self.rows, 1):
                        matrix[i][row] = matrix[i][rank]
                row -= 1
        return (rank)