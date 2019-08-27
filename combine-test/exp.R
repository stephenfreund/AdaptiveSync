#!/usr/bin/env Rscript

library(ggplot2)
library(plyr)
library(Hmisc)
library(dplyr)
library(data.table)

args = commandArgs(trailingOnly=TRUE)

files <- args

data = do.call(rbind, lapply(files, function(x) read.table(x, ,head=FALSE,sep="\t")))


#data <- read.table(args[1],head=FALSE,sep="\t")
colnames(data) <- c('Lock', 'Mode', 'Threads', 'WritePercent', 'WriteDelay', 'ReadDelay', 'OutsideDelay', 'ExpFreq', 'ExpTime', 'Throughput')

data <- data[data$Lock != 'NoLock', ]

data$Throughput = data$Throughput / 1000000 # data$Base
data$WritePercent <- data$WritePercent

data$NewLock <- interaction(data$Lock,data$ExpFreq)
# data2 = data
# data2$Mode <- NULL

 ggplot(data, aes(x=Threads, y=Throughput, color=NewLock, shape=Mode),group=interaction(NewLock, Mode)) + 
   theme_bw() + 
  facet_grid(ReadDelay ~ WritePercent) + #, scales="free") + 
   stat_summary(data=data, fun.y = mean, geom="line") +
 #  scale_y_log10() + 
   geom_point() 



ggplot(data, aes(x=ReadDelay, y=Throughput, color=NewLock)) +
  theme_bw() + 
  facet_grid(Threads ~ WritePercent) + # , scales="free") + 
  geom_point(data=data, aes(shape=Mode)) +
  scale_shape_manual(values = c(4, 1,5)) +
  stat_summary(data=data, fun.y=mean, geom="line", aes(x=ReadDelay, y=Throughput, color=NewLock)) 


ggplot(data, aes(x=ExpFreq, y=Throughput, color=Lock)) +
  theme_bw() + 
  facet_grid(Threads ~ WritePercent )+ #, scales="free") + 
  geom_point(data=data, aes(shape=Mode)) +
  scale_shape_manual(values = c(4, 1,5)) +
  stat_summary(data=data, fun.y=mean, geom="line", aes(x=ExpFreq, y=Throughput, color=Lock)) 



ggplot(data, aes(x=WritePercent, y=Throughput, color=NewLock)) +
  theme_bw() + 
  facet_grid(Threads ~ ReadDelay)+ #, scales="free") + 
#  scale_y_continuous(limits = c(0,4))+
  geom_point(data=data, aes(shape=Mode)) +
  scale_shape_manual(values = c(4, 1,5)) +
  stat_summary(data=data, fun.y=mean, geom="line", aes(x=WritePercent, y=Throughput, color=NewLock)) 




# ggplot(data, aes(x=WritePercent, y=Throughput, color=NewLock)) + # ,group=interaction(NewLock, Mode)) + 
#   theme_bw() + 
#   facet_grid(Threads ~ ReadDelay+WriteDelay)+ #, scales="free") + 
# #  stat_summary(fun.data = "mean_cl_normal", fun.y = mean, geom="smooth") +
# #  scale_y_log10() + 
#   geom_point(data=data, aes(shape=Mode)) +
#   geom_smooth(data=data, method="loess",aes(x=WritePercent, y=Throughput, color=NewLock)) 



# ggplot(data, aes(x=Threads, y=Throughput, color=NewLock)) + # ,group=interaction(NewLock, Mode)) + 
#   theme_bw() + 
#   facet_grid(WritePercent ~ ReadDelay+WriteDelay, scales="free") + 
#   #  stat_summary(fun.data = "mean_cl_normal", fun.y = mean, geom="smooth") +
#   #  scale_y_log10() + 
#   geom_point(data=data, aes(shape=Mode)) +
#   geom_smooth(data=data, method="loess",aes(x=Threads, y=Throughput, color=NewLock)) 


# ggplot(data, aes(x=ReadDelay, y=Throughput, color=NewLock)) + # ,group=interaction(NewLock, Mode)) + 
#   theme_bw() + 
#   facet_grid(WritePercent ~ Threads+WriteDelay, scales="free") + 
#   #  stat_summary(fun.data = "mean_cl_normal", fun.y = mean, geom="smooth") +
#   #  scale_y_log10() + 
#   geom_point(data=data, aes(shape=Mode)) +
#   geom_smooth(data=data, method="loess",aes(x=ReadDelay, y=Throughput, color=NewLock)) 


# # 
# # ggplot(data, aes(x=ReadDelay, y=Throughput, color=NewLock, shape=Mode),group=interaction(NewLock, Mode)) + 
# #   theme_bw() + 
# # #  facet_grid( . ~ , scales="free") + 
# #   stat_summary(fun.y = mean, geom="line") +
# #   geom_point() 
# # 

