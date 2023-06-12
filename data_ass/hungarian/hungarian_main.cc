#include <iostream>
#include <vector>
#include <utility>
#include <limits>
using namespace std;

#define INF (std::numeric_limits<int>::max())

// Given the cost matrix "vector<vector<int>> A {...} n x m;"
// Find the maximum matching "vector<pair<int,int>>result;" with all pairs
// As well as total cost "int C;" with the minimum assignment cost.
// Ref: http://zafar.cc/2017/7/19/hungarian-algorithm/
// Ref: https://en.wikipedia.org/wiki/Hungarian_algorithm
// Ref: https://brilliant.org/wiki/matching-algorithms/
// Ref: https://brilliant.org/wiki/hungarian-matching/
void
hungarian_match(const std::vector<vector<int>>& A, std::vector<pair<int, int>>& result,
        int& C)
{
    std::cout << "start hungarian!" << std::endl;

    int n = A.size(); // n rows, aka workers
    int m = A[0].size(); // m cols, aka jobs

    /**
     * \brief Major state variables.
     * \vars  u and v maintain the potentials for rows and cols, resp.
     *        p[j] stores the row which matches with the col j except for j == 0
     *        way[j]
     * \note We use one-based indexing for convenience!
     *
     */
    vector<int> u(n + 1, 0), v(m + 1, 0), p(m + 1, 0), way(m + 1, 0);

    for (int i = 1; i <= n; ++i)
    {
        p[0] = i; // p[0] stores the current row
        int j0 = 0; // previously found column

        vector<int> minv(m + 1, INF); // minimum positive potential gain for each column
        vector<char> used(m + 1, false);  // used columns, aka assigned columns

        do
        {
            used[j0] = true; // mark used column
            int i0 = p[j0]; // a new used row
            int delta = INF;
            int j1; // The column where the delta is reached.

            /* For row i0, update minv for each column */
            for (int j = 1; j <= m; ++j)
            {
                if (!used[j])
                {
                    /* Note the conversion between zero-based and one-based indexing. */
                    int cur = A[i0 - 1][j - 1]- u[i0] - v[j];
                    if (cur < minv[j])
                    {
                        minv[j] = cur,  way[j] = j0;
                    }

                    /* At column j1, delta reaches the current min. */
                    if (minv[j] < delta)
                    {
                        delta = minv[j],  j1 = j;
                    }
                }
            }

            for (int j = 0; j <= m; ++j)
            {
                if (used[j])
                {
                    u[p[j]] += delta,  v[j] -= delta;
                }
                else
                {
                    minv[j] -= delta;
                }
            }

            /* udpate j0 to the column delta achieves the mininum. */
            j0 = j1;

        } while (p[j0] != 0); // zero as flag

        do
        {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0); // zero as flag
    }

    for (int i = 1; i <= m; ++i)
    {
        result.push_back(make_pair(p[i], i));
    }

    C = -v[0];

    return;
}

int
main()
{
    std::vector<std::vector<int>> A;

            /* [[60, 65, 75, 32, 93], */
            /*  [95, 41, 6, 33, 46], */
            /*  [18, 11, 32, 65, 5], */
            /*  [13, 40, 69, 41, 58], */
            /*  [26, 28, 54, 88, 10]] */
    std::vector<int> row_1 = {60, 65, 75, 32, 93};
    std::vector<int> row_2 = {95, 41, 6, 33, 46};
    std::vector<int> row_3 = {18, 11, 32, 65, 5};
    std::vector<int> row_4 = {13, 40, 69, 41, 58};
    std::vector<int> row_5 = {26, 28, 54, 88, 10};

    A.push_back(row_1);
    A.push_back(row_2);
    A.push_back(row_3);
    A.push_back(row_4);
    A.push_back(row_5);


    std::cout << "A size " << A.size() << std::endl;
    std::cout << "A[0] size " << A[0].size() << std::endl;

    std::vector<std::pair<int, int>> res;
    int C;

    hungarian_match(A, res, C);

    std::cout <<  "min cost: " << C << std::endl;

    return 0;
}
