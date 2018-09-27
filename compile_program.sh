gcc -o main main.c LineNetwork.c StationWait.c Train.c -fopenmp

> output.txt

prevtime=10
time=10
prevtrain=10
train=10

while [ $train -le 21 ]
do
	((train+=1))
	perf stat -o output.txt --append ./main < input.txt 

	sed -i "16s,$prevtrain,$train," input.txt
	sed -i "16s,$prevtrain,$train," input.txt
	sed -i "16s,$prevtrain,$train," input.txt
	prevtrain=$train
done

##To reset the trains back to the original
train=10
	sed -i "16s,$prevtrain,$train," input.txt
	sed -i "16s,$prevtrain,$train," input.txt
	sed -i "16s,$prevtrain,$train," input.txt
prevtrain=$train


##To vary the time in the input file

##while [ $time -le 160 ]
##do
	##((time+=5))
	##perf stat -o output.txt --append ./main < input.txt 

	##sed -i "15s,$prevtime,$time," input.txt
	##oldtime=$time
##done

##To reset the time back to the original 
##train=10
	##sed -i "15s,$prevtime,$time," input.txt
##prevtime=$time
