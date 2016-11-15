pdf('_results/plots_bfs.pdf-1.pdf', height=8.000000, width=13.000000)
error.bar <- function(x, y, upper, lower=upper, length=0.1,...){ 
 if(length(x) != length(y) | length(y) !=length(lower) | length(lower) != length(upper)) 
 stop("vectors must be same length") 
 arrows(x,y+upper, x, y-lower, angle=90, code=3, length=length, ...) 
 } 
 
cols <- as.vector(c('#000000','#FF0000','#0000FF','#00FF00','#FFFF66'))
data0 <- matrix(c(-36.813911,-2.656704,-19.785727,-10.131234,-34.024976,-11.378556,-34.896938,-76.127925,6.272440,6.217883,-5.713736,19.601329,13.569845,-49.154229,-22.411522,16.853466,-19.612926,11.548556,-13.006724,-13.129103,-22.809399,-69.904322,4.024436,9.969168,-0.769527,13.621262,4.124169,-42.158771,-37.613068,-8.053134,-21.876620,-10.656168,-35.216138,-13.566740,-34.345576,-81.354711,-88.500080,-5.858171,-71.796845,16.611296,5.277162,-45.070301,-23.799069,17.227065,-21.876620,9.816273,-15.830932,-14.442013,-24.395623,-80.833283,-88.513477,-3.391572,-72.123894,22.591362,5.631929,-49.390006,-2.537982,3.528435,21.548298,-7.349081,-8.126801,23.194748,-3.325134,0.091075,-90.445314,-6.680370,-72.931897,306.644518,-1.152993,-3.954142),ncol=5)
data0err <- matrix(c(1.956266,2.499638,1.040544,2.527329,2.019394,2.336341,1.101286,1.647348,1.893197,3.176892,19.098202,8.457091,3.266580,0.660131,3.072200,3.885660,2.631574,1.445239,1.506910,2.952021,1.668744,0.201282,3.616522,3.340580,15.506656,7.094404,1.525911,4.435090,1.105479,6.079059,0.953073,2.336990,1.555128,3.274974,2.236289,0.229235,0.413911,0.838104,0.668647,5.696488,3.825589,1.198083,2.606004,9.992863,1.249198,8.473098,2.115796,3.568820,1.493110,0.519111,0.255962,1.790468,0.424113,6.661109,3.481082,1.295921,1.939518,3.293787,0.822214,1.462298,2.659221,5.731204,0.374576,3.204537,0.085657,4.121902,0.612608,3.547202,3.244229,2.720788),ncol=5)

colnames(data0) <- c("Seq. neighbor list, oracle guided, kappa := 30usec","Seq. neighbor list, oracle guided, kappa := 100usec","Par. neighbor list, oracle guided, kappa := 30usec","Par. neighbor list, oracle guided, kappa := 100usec","Par. neighbor list PBBS BFS")

rownames(data0) <- c("europe","rgg","twitter","delaunay","usa","livejournal","square-grid","par-chains-100","trees_524k","phases-50-d-5","phases-10-d-2","random-arity-100","trees-512-1024","unbalanced-tree")

table0 <- t(as.table(data0))

par(mar=c(12, 4, 4, 2) + 0.4)
bp <- barplot(table0, beside=TRUE, space=c(0,2), las=2, col=cols, names.arg=colnames(table0), xlab='', ylab='% relative to original PBBS BFS', legend=rownames(table0), args.legend = list(x = 'topleft'), ylim=range(-100.000000,100.000000) )
error.bar(t(bp),data0,data0err)
title(main='', col.main='black', font.main=1, cex.main=1)

dev.off()