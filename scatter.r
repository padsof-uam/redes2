args <- commandArgs(trailingOnly = TRUE)
times = scan(args[1])
n = seq(0,length(times) - 1, 1)
png('scatter.png', 700, 400)
library(car, lib.loc="~/Downloads/")
scatterplot(n, times, xlab='Numero de ping', ylab='Tiempo de respuesta(ms)', boxplots='y', reg.line=FALSE, spread=TRUE)
dev.off()
