#include <cstdlib>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>
#include <bitset>
#include <cassert>

/*
 * 3x3   a: 00           0 = x+y
 * abc   b: 01, 21       1 = x+y
 * bcd   c: 13, 22, 31   4 = x+y
 * cde   d: 23, 32       5 = x+y
 *       e: 33           6 = x+y = n*2
 *  => n*2-1 elements
 *     i = x+y
 *
 * 4x4   a: x+y = 2
 * abcd  b: x+y = 3
 * bcde  .
 * cdef  .
 * defg  f: x+y = 7
 *       g: x+y = 8 = n*2
 *
 *  => n*2-1 elements
 *
 *
 * 3x3   a: 20           (n-x)+y = 1
 * cba   b: 10, 21       (n-x)+y = 2
 * dcb   c: 00, 11, 22   (n-x)+y = 3
 * edc   d: 01, 12       (n-x)+y = 4
 *       e: 02           (n-x)+y = 5
 *
 *  => n*2-1 elements
 *     i = (n-x)+y - 1
 *
 */

struct Field
{
  int n;
  std::vector<bool> x, y, xy, yx;
  std::set<std::pair<int, int>> queens;

  Field(int n)
  : n(n)
  , x(n), y(n), xy(n*2-1), yx(n*2-1)
  {}

  Field(const Field & other)
  : n(other.n)
  , x(other.x)
  , y(other.y)
  , xy(other.xy)
  , yx(other.yx)
  , queens(other.queens)
  {
  }

  bool operator<(const Field & other) const
  {
    assert(this->queens.size() == other.queens.size());

    auto lhs = this->queens.begin();
    auto rhs = other.queens.begin();
    while (lhs != this->queens.end()
        && rhs != other.queens.end())
    {
      if (*lhs == *rhs)
      {
        ++lhs;
        ++rhs;
      }
      else
      {
        return *lhs < *rhs;
      }
    }
    // for all iterations lhs == rhs
    return false;
  }

  bool is_free(int x, int y) const
  {
    return //!this->x[x] &&
           !this->y[y] &&
           !this->xy[x+y] &&
           !this->yx[n-x+y-1];
  }

  Field with_queen_at(int x, int y) const
  {
    Field result = *this;
    result.x[x] = true;
    result.y[y] = true;
    result.xy[x+y] = true;
    result.yx[n-x+y-1] = true;
    result.queens.insert(std::pair<int,int>(x, y));
    return result;
  }
};

typedef std::set<Field> FieldSet;


void depth_first(const Field & field, FieldSet & result)
{
  //std::cout << "descending: " << field.queens.size() << "/" << field.n << std::endl;
  if (field.queens.size() == field.n)
    result.insert(field);

  for (int x = 0; x < field.n; ++x)
  {
    if (field.x[x])
      continue;
    for (int y = 0; y < field.n; ++y)
    {
      if (field.is_free(x, y))
      {
        Field variation = field.with_queen_at(x, y);
        depth_first(variation, result);
      }
    }
  }
}

int main(int argc, char** argv)
{
  if (argc != 2)
    return -1;

  int n = std::atoi(argv[1]);

  std::cout << n << " queens" << std::endl;
  Field field(n);
  FieldSet result;
  depth_first(field, result);

  std::cout << "found " << result.size() << " solutions." << std::endl;

  return 0;
}

