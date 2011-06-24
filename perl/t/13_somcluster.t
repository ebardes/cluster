
my ($last_test,$loaded);

######################### We start with some black magic to print on failure.
use lib '../blib/lib','../blib/arch';

BEGIN { $last_test = 5; $| = 1; print "1..$last_test\n"; }
END   { print "not ok 1  Can't load Algorithm::Cluster\n" unless $loaded; }

use Algorithm::Cluster;
use warnings 'Algorithm::Cluster';
use Data::Dumper;

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
my ($clusterid);
my ($i);

my %params;

%params = (

	transpose =>         0,
	dist      =>       'e',
	data      =>    $data1,
	mask      =>    $mask1,
	weight    =>  $weight1,
	niter     =>       100,
	nxnodes   =>        10,
	nynodes   =>        10,
);

$clusterid = Algorithm::Cluster::somcluster(%params);

$want = scalar(@$data1);           test q( scalar(@$clusterid) );
$want = '2';                       test q( scalar(@{$clusterid->[0]}) );

%params = (

	transpose =>         0,
	dist      =>       'e',
	data      =>    $data2,
	mask      =>    $mask2,
	weight    =>  $weight2,
	niter     =>       100,
	nxnodes   =>        10,
	nynodes   =>        10,
);

$clusterid = Algorithm::Cluster::somcluster(%params);

$want = scalar(@$data2);           test q( scalar(@$clusterid) );
$want = '2';                       test q( scalar(@{$clusterid->[0]}) );


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
