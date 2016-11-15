pdf('_results/chart-1.pdf', height=8.000000, width=6.000000)
error.bar <- function(x, y, upper, lower=upper, length=0.1,...){ 
 if(length(x) != length(y) | length(y) !=length(lower) | length(lower) != length(upper)) 
 stop("vectors must be same length") 
 arrows(x,y+upper, x, y-lower, angle=90, code=3, length=length, ...) 
 } 
 
cols <- as.vector(c('#000000','#FF0000'))
data0 <- matrix(c(-1.246924,-3.763383),ncol=2)
data0err <- matrix(c(3.468044,2.461652),ncol=2)

colnames(data0) <- c("random distribution in square","kuzmin distribution")

rownames(data0) <- c("")

table0 <- t(as.table(data0))

par(mar=c(12, 4, 4, 2) + 0.4)
bp <- barplot(table0, beside=TRUE, space=c(0,2), las=2, col=cols, names.arg=colnames(table0), xlab='', ylab='% relative to original PBBS', legend=rownames(table0), args.legend = list(x = 'topleft'), ylim=range(-40.000000,40.000000) )
error.bar(t(bp),data0,data0err)
title(main='', col.main='black', font.main=1, cex.main=1)

dev.off()