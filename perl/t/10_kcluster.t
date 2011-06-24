
my ($last_test,$loaded);

######################### We start with some black magic to print on failure.
use lib '../blib/lib','../blib/arch';

BEGIN { $last_test = 27; $| = 1; print "1..$last_test\n"; }
END   { print "not ok 1  Can't load Algorithm::Cluster\n" unless $loaded; }

use Algorithm::Cluster;
no  warnings 'Algorithm::Cluster';

$loaded = 1;
print "ok 1\n";

######################### End of black magic.

sub test;  # Predeclare the test function (defined below)

my $tcounter = 1;
my $want     = '';


#------------------------------------------------------
# Data for Tests
# 

#----------
# dataset 1
#
my $weight1 =  [ 1,1,1,1,1 ];
my $data1   =  [
        [ 1.1, 2.2, 3.3, 4.4, 5.5, ], 
        [ 3.1, 3.2, 1.3, 2.4, 1.5, ], 
        [ 4.1, 2.2, 0.3, 5.4, 0.5, ], 
        [ 12.1, 2.0, 0.0, 5.0, 0.0, ], 
];
my $mask1 =  [
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
        [ 1, 1, 1, 1, 1, ], 
];

#----------
# dataset 2
#
my $weight2 =  [ 1,1 ];
my $data2   =  [
	[ 1.1, 1.2 ],
	[ 1.4, 1.3 ],
	[ 1.1, 1.5 ],
	[ 2.0, 1.5 ],
	[ 1.7, 1.9 ],
	[ 1.7, 1.9 ],
	[ 5.7, 5.9 ],
	[ 5.7, 5.9 ],
	[ 3.1, 3.3 ],
	[ 5.4, 5.3 ],
	[ 5.1, 5.5 ],
	[ 5.0, 5.5 ],
	[ 5.1, 5.2 ],
];
my $mask2 =  [
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
	[ 1, 1 ],
];


#------------------------------------------------------
# Tests
# 
my ($clusters, $centroids, $error, $found);
my ($i,$j);

my %params = (

	nclusters =>         3,
	transpose =>         0,
	method    =>       'a',
	dist      =>       'e',
);

#----------
# test dataset 1
#
($clusters, $error, $found) = Algorithm::Cluster::kcluster(

	%params,
	data      =>    $data1,
	mask      =>    $mask1,
	weight    =>  $weight1,
        npass     =>       100,
);

#$i=0;$j=0;
#foreach(@{$clusters}) {
#	printf("%2d: %2d\n",$i++,$_);
#}

#----------
# Make sure that the length of @clusters matches the length of @data
$want = scalar @$data1;       test q( scalar @$clusters );

#----------
# Test the cluster coordinates
$want = '1';       test q( $clusters->[ 0] != $clusters->[ 1] );
$want = '1';       test q( $clusters->[ 1] == $clusters->[ 2] );
$want = '1';       test q( $clusters->[ 2] != $clusters->[ 3] );

# Test the within-cluster sum of errors
$want = '  1.300';       test q( sprintf "%7.3f", $error);


#----------
# test dataset 2
#
$i=0;$j=0;
($clusters, $error, $found) = Algorithm::Cluster::kcluster(

	%params,
	data      =>    $data2,
	mask      =>    $mask2,
	weight    =>  $weight2,
        npass     =>       100,
);

#$i=0;$j=0;
#foreach(@{$clusters}) {
#	printf("%2d: %2d\n",$i++,$_);
#}


#----------
# Make sure that the length of @clusters matches the length of @data
$want = scalar @$data2;       test q( scalar @$clusters );

#----------
# Test the cluster coordinates
$want = '1';       test q( $clusters->[ 0] == $clusters->[ 3] );
$want = '1';       test q( $clusters->[ 0] != $clusters->[ 6] );
$want = '1';       test q( $clusters->[ 0] != $clusters->[ 9] );
$want = '1';       test q( $clusters->[11] == $clusters->[12] );

# Test the within-cluster sum of errors
$want = '  1.012';       test q( sprintf "%7.3f", $error);

#----------
# test kcluster with initial cluster assignments
#
$initialid = [0,1,2,0,1,2,0,1,2,0,1,2,0];

($clusters, $error, $found) = Algorithm::Cluster::kcluster(

	%params,
	data      =>    $data2,
	mask      =>    $mask2,
	weight    =>  $weight2,
        npass     =>         1, 
        initialid => $initialid, 
);

#----------
# Test the cluster coordinates
$want = '2';       test q( $clusters->[ 0] );
$want = '2';       test q( $clusters->[ 1] );
$want = '2';       test q( $clusters->[ 2] );
$want = '2';       test q( $clusters->[ 3] );
$want = '2';       test q( $clusters->[ 4] );
$want = '2';       test q( $clusters->[ 5] );
$want = '0';       test q( $clusters->[ 6] );
$want = '0';       test q( $clusters->[ 7] );
$want = '2';       test q( $clusters->[ 8] );
$want = '1';       test q( $clusters->[ 9] );
$want = '1';       test q( $clusters->[10] );
$want = '1';       test q( $clusters->[11] );
$want = '1';       test q( $clusters->[12] );

# Test the within-cluster sum of errors
$want = '  3.036';       test q( sprintf "%7.3f", $error);
   
$want = '1';       test q($found);
                 
#------------------------------------------------------
# Test function
# 
sub test {
	$tcounter++;

	my $string = shift;
	my $ret = eval $string;
	$ret = 'undef' if not defined $ret;

	if("$ret" =~ /^$want$/sm) {

		print "ok $tcounter\n";

	} else {
		print "not ok $tcounter\n",
		"   -- '$string' returned '$ret'\n", 
		"   -- expected =~ /$want/\n"
	}
}

__END__



