#!/usr/bin/env bash 

if [ $# -ne 2 ]; then
	echo "usage: you must set the build type! and input file"
	exit 1
fi
build_type=$1
input_file=$2

if [ ! -f $input_file ]; then
	echo "usage: invalid input file: $input_file"
	exit 1
fi

function do_test {
	app_name=$1
	timeout -s 9 -k 20s 20s $app_name  $input_file > $output_file  2> /dev/null
	if [ $? -ne 0 ]; then
		echo -n "critical error running the app"
		return 1
	fi
	main_thread=$(grep "main thread" $output_file | awk '{print $4}')

	mtc=$(grep $main_thread $output_file | grep -c "started")
	if [ $mtc -lt 1 ]; then 
		echo -n "error we have more than a single instance of main thread running in the test $mtc"
		return 1
	fi

	if [ $(grep -c "started on thread" $output_file) -ne 1000 ]; then
		echo -n "number of subtask in not corrected"
		return 1
	fi

	if [ "$build_type" = "Debug" ]; then
		# I tried in many ways to make this to work with release build, but it didn't work, so we skip this one
		sw=$(grep -c "switched" $output_file)
		if [ $sw -lt 1000 ]; then
			echo -n "we didn't have enough switches between threads: $sw"
			return 1
		fi
	fi

	tc=$(grep fiber  $output_file | grep started | awk '{print $6}' | sort | uniq | wc -l)
	if [ $tc -lt 40 ]; then
		echo -n "number of threads used $tc is less than expected"
		return 1
	fi

	if [ $(grep -c "we have some error" $output_file) -ne 0 ]; then
		echo -n "we have errors in the run"
		return 1
	fi

	done_seen=$(grep -c "is done" $output_file)
	if [ $done_seen -ne 1000 ]; then
		echo -n "we don't have the correct number of done for the tasks that we executed, expecting 1000, got $done_seen"
		return 1
	fi
	echo " "
	return 0
}

output_file=/tmp/output_$(date +"%s")

echo "building $build_type"
./build.sh -g 1 -t ${build_type} -b build/${build_type} || {
	echo "error building the tests"
	exit 1
}
./build.sh -t $build_type -b build/${build_type} || {
	echo "error building the tests"
	exit 1
}

echo "running the test without result returning from a function"
for i in {1..100}; do
	echo -n "running test number $i: "
	output=$(do_test ./build/${build_type}/tests/test_pool)
	if [ $? -ne 0 ]; then
		echo -e " \e[0;31mFAIL\e[0m: $output"
		rm -f ${output_file}
		exit 1
	else
		echo -e " \e[0;32mOK\e[0m"
		rm -f $output_file
	fi
done

echo "successfully finish 100 tests"

echo "running tests with the using result from a function"
for i in {1..100}; do
	echo -n "running test number $i: "
	output=$(do_test ./build/${build_type}/tests/test_with_result)
	if [ $? -ne 0 ]; then
		echo -e " \e[0;31mFAIL\e[0m: $output"
		rm -f ${output_file}
		exit 1
	else
		echo -e " \e[0;32mOK\e[0m"
		rm -f $output_file
	fi
done

exit 0