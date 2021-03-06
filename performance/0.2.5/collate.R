library(dplyr)
library(ggplot2)

t1=read.table("test1_time_mem.txt",header=T)
t2=read.table("test2_time_mem.txt",header=T)
t3=read.table("test3_time_mem.txt",header=T)
t4=read.table("test4_time_mem.txt",header=T)
t5=read.table("test5_time_mem.txt",header=T)
t6=read.table("test6_time_mem.txt",header=T)

t1.groups = group_by(t1,thetas,version)
t2.groups = group_by(t2,thetas,version)
t3.groups = group_by(t3,thetas,version)
t4.groups = group_by(t4,thetas,version)
t5.groups = group_by(t5,thetas,version)
t6.groups = group_by(t6,thetas,version)

t1.summ = summarise(t1.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))
t2.summ = summarise(t2.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))
t3.summ = summarise(t3.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))
t4.summ = summarise(t4.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))
t5.summ = summarise(t5.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))
t6.summ = summarise(t6.groups,mm=mean(mems),mt=mean(times),mdt=median(times),mint=min(times),maxt=max(times))

t1.summ.plot=ggplot(t1.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t1_time.png",scale=0.5)
t1.summ.mem.plot=ggplot(t1.summ,aes(x=thetas,y=mm/(4*1024))) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean peak RAM use (megabytes)")
ggsave("t1_mem.png",scale=0.5)
t2.summ.plot=ggplot(t2.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t2_time.png",scale=0.5)
t2.summ.mem.plot=ggplot(t2.summ,aes(x=thetas,y=mm/(4*1024))) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean peak RAM use (megabytes)")
ggsave("t2_mem.png",scale=0.5)
t3.summ.plot=ggplot(t3.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t3_time.png",scale=0.5)
t3.summ.mem.plot=ggplot(t3.summ,aes(x=thetas,y=mm/(4*1024))) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean peak RAM use (megabytes)")
ggsave("t3_mem.png",scale=0.5)
t4.summ.plot=ggplot(t4.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t4_time.png",scale=0.5)
t4.summ.mem.plot=ggplot(t4.summ,aes(x=thetas,y=mm/(4*1024))) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean peak RAM use (megabytes)")
ggsave("t4_mem.png",scale=0.5)
t5.summ.plot=ggplot(t5.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t5_time.png",scale=0.5)
t5.summ.mem.plot=ggplot(t5.summ,aes(x=thetas,y=mm/(4*1024))) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean peak RAM use (megabytes)")
ggsave("t5_mem.png",scale=0.5)
t6.summ.plot=ggplot(t6.summ,aes(x=thetas,y=mt/60)) + geom_point(aes(color=version,shape=version)) + geom_line(aes(color=version)) + xlab(expression(paste(theta," = ",rho))) + ylab("Mean run time (minutes)")
ggsave("t6_time.png",scale=0.5)

png("t3_perf.png")
plot(unique(t3$thetas),t3.summ$mt[t3.summ$version=="0.2.5"]/t3.summ$mt[t3.summ$version=="0.2.4"],ylim=c(0,1),
     xlab=expression(paste(theta," = ",rho)),
     ylab="Mean run-time v0.2.5/Mean run-time v0.2.4",main="Relative run-time decrease")
lines(unique(t3$thetas),t3.summ$mt[t3.summ$version=="0.2.5"]/t3.summ$mt[t3.summ$version=="0.2.4"])
for( i in seq(0.25,1,0.25) )
    {
        abline(i,0,lty="dashed")
    }
dev.off()

png("t4_perf.png")
plot(unique(t4$thetas),t4.summ$mt[t4.summ$version=="0.2.5"]/t4.summ$mt[t4.summ$version=="0.2.4"],ylim=c(0,1),
     xlab=expression(paste(theta," = ",rho)),
     ylab="Mean run-time v0.2.5/Mean run-time v0.2.4",main="Relative run-time decrease")
lines(unique(t4$thetas),t4.summ$mt[t4.summ$version=="0.2.5"]/t4.summ$mt[t4.summ$version=="0.2.4"])
for( i in seq(0.25,1,0.25) )
    {
        abline(i,0,lty="dashed")
    }
abline(h=0.4,col="red",lty="dotdash")
dev.off()

png("t5_perf.png")
plot(unique(t5$thetas),t5.summ$mt[t5.summ$version=="0.2.5"]/t5.summ$mt[t5.summ$version=="0.2.4"],ylim=c(0,1),
     xlab=expression(paste(theta," = ",rho)),
     ylab="Mean run-time v0.2.5/Mean run-time v0.2.4",main="Relative run-time decrease")
lines(unique(t5$thetas),t5.summ$mt[t5.summ$version=="0.2.5"]/t5.summ$mt[t5.summ$version=="0.2.4"])
lines(unique(t5$thetas),t5.summ$mt[t5.summ$version=="0.2.5_test"]/t5.summ$mt[t5.summ$version=="0.2.4"],col="red")
for( i in seq(0.25,1,0.25) )
    {
        abline(i,0,lty="dashed")
    }
abline(h=0.4,col="red",lty="dotdash")
dev.off()

png("t6_perf.png")
plot(unique(t6$thetas),t6.summ$mt[t6.summ$version=="0.2.5"]/t6.summ$mt[t6.summ$version=="0.2.4"],ylim=c(0,1),
     xlab=expression(paste(theta," = ",rho)),
     ylab="Mean run-time v0.2.5/Mean run-time v0.2.4",main="Relative run-time decrease")
lines(unique(t6$thetas),t6.summ$mt[t6.summ$version=="0.2.5"]/t6.summ$mt[t6.summ$version=="0.2.4"])
lines(unique(t6$thetas),t6.summ$mt[t6.summ$version=="0.2.5_test"]/t6.summ$mt[t6.summ$version=="0.2.4"],col="red")
for( i in seq(0.25,1,0.25) )
    {
        abline(i,0,lty="dashed")
    }
abline(h=0.4,col="red",lty="dotdash")
dev.off()


