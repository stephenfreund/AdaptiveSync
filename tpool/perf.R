#!/usr/bin/env Rscript

library(ggplot2)
library(plyr)
library(Hmisc)
library(dplyr)
library(data.table)

args = commandArgs(trailingOnly=TRUE)
#args = c("data2.tsv")

files <- args

data = do.call(rbind, lapply(files, function(x) read.table(x, ,head=FALSE,sep=",")))

colnames(data) <- c('Threads', 'Throughput')

ggplot(data, aes(x=Threads, y=Throughput)) +
  stat_smooth() +
  geom_point(shape=1)      # Use hollow circles
