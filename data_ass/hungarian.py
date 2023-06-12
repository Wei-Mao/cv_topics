import numpy as np
import numpy.typing as npt
import copy
from scipy.optimize import linear_sum_assignment
import networkx as nx
import matplotlib.pyplot as plt

# ref: https://python.plainenglish.io/hungarian-algorithm-introduction-python-implementation-93e7c0890e15

# ref: https://www.hungarianalgorithm.com/examplehungarianalgorithm.php

# ref: https://leimao.github.io/blog/Hungarian-Matching-Algorithm/

# ref: https://en.wikipedia.org/wiki/Hungarian_algorithm


# ref: https://transport-systems.imperial.ac.uk/tf/60008_21/n2_5_hungarian_algorithm.html

# #The matrix who you want to find the maximum sum profit_matrix = np.random.randint(10, size=(5, 5)) #Using the maximum value of the profit_matrix to get the corresponding cost_matrix max_value = np.max(profit_matrix) #Using the cost matrix to find which positions are the answer
# cost_matrix = max_value - profit_matrix
#
# print(f"The profit matrix is:\n", profit_matrix, f"\nThe cost matrix is:\n", cost_matrix)

def scipy_lap():
    max_int = 100
    n = 5
    # cost_matrix = np.random.randint(max_int, size=(n, n))

    cost_matrix = np.array(
            [[60, 65, 75, 32, 93],
             [95, 41, 6, 33, 46],
             [18, 11, 32, 65, 5],
             [13, 40, 69, 41, 58],
             [26, 28, 54, 88, 10]]
    )

    print(f'cost matrix:\n {cost_matrix}')

    scp_assign = linear_sum_assignment(cost_matrix)

    scp_total = 0

    for i in range(len(scp_assign[0])):
        scp_total += cost_matrix[scp_assign[0][i], scp_assign[1][i]]

    print(f'scp_total: {scp_total}')

def draw_network(cost_matrix, assignment):

    x_diff = 10

    y_min = 0
    y_max = 5

    G = nx.Graph()

    for i in range(len(assignment)):
        G.add_node(f"r_{i}", pos=(y_min, i*x_diff))
        G.add_node(f"c_{i}", pos=(y_max, i*x_diff))

    pos=nx.get_node_attributes(G,'pos')

    for i in range(len(assignment)):
        for j in range(len(assignment)):
            val = [i, j]
            if val in assignment:
                c = 'r'
                w = 4
            else:
                c = 'k'
                w = 2
            G.add_edge(f"r_{i}", f"c_{j}", color=c, weight=w)

    edges = G.edges()
    colors = [G[u][v]['color'] for u,v in edges]
    weights = [G[u][v]['weight'] for u,v in edges]
    nx.draw_networkx(G,pos,with_labels=True, node_size=600, font_color='w', edge_color=colors, width=weights)

    # nx depends on matplotlib for visualization
    plt.show()

def calc_costs(cost_matrix, assignment):
    total = 0
    for a in assignment:
        total += cost_matrix[a[0], a[1]]
    return total

def clean_assignment(row, columns):
    assignments = []
    # create pairs
    text = "The final assignment is "
    for i in range(len(row)):
        assignments.append([row[i], columns[i]])
        if i > 0:
            text += ", "
        text += f"({row[i]}, {columns[i]})"
    print(text)

    return assignments


def run_assignment(cost_matrix):
    row, columns = linear_sum_assignment(cost_matrix)
    assignments = clean_assignment(row, columns)
    total_cost = calc_costs(cost_matrix, assignments)
    print(f'The final assignment is {assignments}')
    print(f"The total is {total_cost}.")

    return total_cost
    # draw_network(cost_matrix, assignments)

def run_ass():
    while True:
        max_int = 100
        n = 5
        cost_matrix = np.random.randint(max_int, size=(n, n))
        cost_matrix_cp = copy.deepcopy(cost_matrix)

        total_cost_gt = run_assignment(cost_matrix)
        total_cost_hg = hungarian_algorithm_main(cost_matrix_cp)

        if total_cost_gt != total_cost_hg:
            print("Test Fail!")
            break
        else:
            print("Success!")


#######################################################################
# We find the minimum value at each row, and then subtract that value to every element of the row.
# Then we repeat the process using the columns instead
#######################################################################
def hungarian_step(mat: npt.NDArray):
    # The for-loop iterates through every column in the matrix so we subtract this value to
    # every element of the row
    for row_num in range(mat.shape[0]):
        mat[row_num] = mat[row_num] - np.min(mat[row_num])

    # We repeat the process for the columns
    for col_num in range(mat.shape[1]):
        mat[:,col_num] = mat[:,col_num] - np.min(mat[:,col_num])

    return mat

######################################################################
# Find the minimum number of rows to mark the zero value at that row.
# The zero element in mat corresponds to the True in zero_mat
######################################################################
def min_zeros(zero_mat, mark_zero):
    # min_row = [number of zeros, row index number]
    min_row = [99999, -1]

    for row_num in range(zero_mat.shape[0]):
        num_of_zeros = np.sum(zero_mat[row_num] == True)
        if num_of_zeros > 0 and num_of_zeros < min_row[0]:
            min_row = [num_of_zeros, row_num]
        # if np.sum(zero_mat[row_num] == True) > 0 and min_row[0] > np.sum(zero_mat[row_num] == True):
        #     min_row = [np.sum(zero_mat[row_num] == True), row_num]

    # Marked the specific row and column as False
    zero_index = np.where(zero_mat[min_row[1]] == True)[0][0] # return col_idx
    mark_zero.append((min_row[1], zero_index))
    zero_mat[min_row[1], :] = False
    zero_mat[:, zero_index] = False


######################################################################
# We can image zero element position for each marked row or marked col represents an assignment.
# Hence, we need to # of marked rows + # of marked cols == # of workers (== # of jobs)
######################################################################
def mark_matrix(mat):
    #Transform the matrix to boolean matrix(0 = True, others = False)
    cur_mat = mat
    zero_bool_mat = (cur_mat == 0)
    zero_bool_mat_copy = zero_bool_mat.copy()

    # Recording possible answer positions by marked_zero
    marked_zero = [] # (row, col) for possible assignment
    while (True in zero_bool_mat_copy):
        min_zeros(zero_bool_mat_copy, marked_zero)

    # Recording the row and column indexes seperately.
    marked_zero_row = []
    marked_zero_col = []
    for i in range(len(marked_zero)):
        marked_zero_row.append(marked_zero[i][0])
        marked_zero_col.append(marked_zero[i][1])

    # mark rows not containing zeros after crossing out zeros by horizontal or vertical lines.
    # That means the workers which has not been assigned jobs yet.
    non_marked_row = list(set(range(cur_mat.shape[0])) - set(marked_zero_row))

    # mark columns with zeros
    marked_cols = []
    check_switch = True

    # Switch assignment until being unable to switch any further.
    while check_switch:
        check_switch = False
        for i in range(len(non_marked_row)):
            row_array = zero_bool_mat[non_marked_row[i], :]
            for j in range(row_array.shape[0]):
                # if the job has not been assigned, assign it to the worker at position of zero.
                if row_array[j] == True and j not in marked_cols:
                    marked_cols.append(j)
                    check_switch = True

        for row_num, col_num in marked_zero:
            # Switch assignment and make the previously assigned worker unassigned
            if row_num not in non_marked_row and col_num in marked_cols:
                non_marked_row.append(row_num)
                check_switch = True
                # update marked_zero


    # mark rows with zeros, assigned workers
    marked_rows = list(set(range(mat.shape[0])) - set(non_marked_row))

    # marked_rows mean the assigned workers
    # marked_cols mean the assigned jobs
    # worker and job have one-to-one correspondce relation.
    return(marked_zero, marked_rows, marked_cols)

######################################################################
# cover_rows means marked_rows
# cover_cols means marked_cols
######################################################################
def adjust_matrix(mat, cover_rows, cover_cols):
    cur_mat = mat
    non_zero_element = []

    # Find the minimum value of an element not in a marked column/row
    # min_num = float('inf')
    for row in range(len(cur_mat)):
        if row not in cover_rows:
            for i in range(len(cur_mat[row])):
                if i not in cover_cols:
                    # min_num = min(min_num, cur_mat[row][i])
                    non_zero_element.append(cur_mat[row][i])

    min_num = min(non_zero_element)

    # Substract min_num from all values not in a marked row/column
    for row in range(len(cur_mat)):
        if row not in cover_rows:
            for i in range(len(cur_mat[row])):
                if i not in cover_cols:
                    cur_mat[row, i] -= min_num

    # add to all values in marked rows/column
    for row in range(len(cover_rows)):
        for col in range(len(cover_cols)):
            cur_mat[cover_rows[row], cover_cols[col]] +=  min_num

    return cur_mat

def hungarian_algorithm(cost_matrix):
    n = cost_matrix.shape[0]
    cur_mat = copy.deepcopy(cost_matrix)

    # Substract the row min and column min.
    cur_mat = hungarian_step(cur_mat)

    count_zero_lines = 0

    ans_pos = None
    while count_zero_lines < n:
        # Mark the possible optimal assignments
        ans_pos, marked_rows, marked_cols = mark_matrix(cur_mat)
        count_zero_lines = len(marked_rows) + len(marked_cols)

        if count_zero_lines < n:
            cur_mat = adjust_matrix(cur_mat, marked_rows, marked_cols)

    min_zeros(cur_mat == 0, ans_pos)
    return ans_pos

def hungarian_algorithm_main(cost_matrix):
    # The implementation is wrong!
    assignment = hungarian_algorithm(cost_matrix)
    print(f"The final assignment of HG is: {assignment}")
    print(cost_matrix)
    total_cost = calc_costs(cost_matrix, assignment)
    print(f'Total_cost hungarian: {total_cost}')

    return total_cost

if __name__ == '__main__':
    run_ass()

