./bench.pbench compare -benchmark comparison_sort,blockradix_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours,ray_cast,delaunay -size large -runs 30 -proc 40 -exts unks100
#./bench.pbench compare -benchmark delaunay_refine -size large -runs 6 -proc 40 -exts unks100
./bench.pbench bfs -proc 40 -runs 30

#./bench.pbench compare -benchmark comparison_sort,blockradix_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours,ray_cast -size large -runs 6 -proc 39,40 -exts unks50,unks100,unks200
#./bench.pbench compare -benchmark delaunay -size large -runs 6 -proc 39,40 -exts unks50,unks100,unks200
#./bench.pbench compare -benchmark delaunay_refine -size large -runs 6 -proc 39,40 -exts unks50,unks100,unks200
#./bench.pbench compare -benchmark comparison_sort,blockradix_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours,ray_cast,bfs -size large -runs 3 -proc 1,10,20,30,39,40 -exts unks100,unks
#./bench.pbench compare -benchmark delaunay -size large -runs 3 -proc 1,10,20,30,39,40 -exts unks100,unks
#./bench.pbench compare -benchmark delaunay_refine -size large -runs 3 -proc 1,10,20,30,39,40 -exts unks100,unks
