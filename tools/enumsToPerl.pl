#!/usr/bin/perl

# enumsToPerl:
# This Perl script creates a Perl version of the enum's that are in
# the C/C++ header file supplied as the first command-line argument.
# The second command-line argument specifies the filepath to the
# module file that is to be created.
# Sample usage:
# perl enumsToPerl MyEnums.h MyEnums.pm
#
# Cameron Hayne (macdev@hayne.net)  May 2009

use strict;
use warnings;
use File::Basename;

# getEnumsFromString:
sub getEnumsFromString($)
{
    my ($str) = @_;
    
    # strip C & C++ style comments
    # (regex from: http://perldoc.perl.org/perlfaq6.html)
    $str =~ s#/\*[^*]*\*+([^/*][^*]*\*+)*/
             |//[^\n]*
             |q("(\\.|[^"\\])*" | '(\\.|[^'\\])*' | .[^/"'\\]*)
             #defined $2 ? $2 : ""#gsex;
    
    my @enums = ();
    while ($str =~ /(enum\s+(\w+)\s*{([^}]*)}\s*;)/g)
    {
        my $enumDecl = $1;
        my $enumName = $2;
        my $enumBody = $3;
        
        my $enum = {};
        $enum->{name} = $enumName;
        $enum->{pairs} = [];
        push(@enums, $enum);
        
        my $prevValue = -1;
        while ($enumBody =~ /(\w+)\s*(?:=\s*(.+))?,?/g)
        {
            my $name = $1;
            my $value = $2;
            if (defined($value))
            {
                $value = eval($value);
            }
            else
            {
                $value = $prevValue + 1;
            }
            $prevValue = $value;
            
            push(@{$enum->{pairs}}, [$name, $value]);
        }
    }
    
    return @enums;
}

# writePerlCodeForEnums:
sub writePerlCodeForEnums($$@)
{
    my ($filehandle, $useHex, @enums) = @_;
    
    print $filehandle "# Enums:\n";
    print $filehandle "# -----------------------------------------\n";
    
    my $format = $useHex ? "\t'%s' => 0x%02x,\n"
                         : "\t'%s' => %d,\n";

    foreach my $enum (@enums)
    {
        my $enumName = $enum->{name};
        print $filehandle "our %$enumName = (\n";
        foreach my $pair (@{$enum->{pairs}})
        {
            my $name = $pair->[0];
            my $value = $pair->[1];
            
            printf $filehandle ($format, $name, $value);
        }
        print $filehandle ");\n";
        print $filehandle "\n";
    }
}

# writePerlCodeForReverseEnums:
sub writePerlCodeForReverseEnums($$@)
{
    my ($filehandle, $useHex, @enums) = @_;
    
    print $filehandle "# Reverse Enums:\n";
    print $filehandle "# -----------------------------------------\n";
    
    my $format = $useHex ? "\t0x%02x => '%s',\n"
                         : "\t%d => '%s',\n";

    foreach my $enum (@enums)
    {
        my $enumName = $enum->{name} . "REVERSE";
        print $filehandle "our %$enumName = (\n";
        foreach my $pair (@{$enum->{pairs}})
        {
            my $name = $pair->[0];
            my $value = $pair->[1];
            printf $filehandle ($format, $value, $name);
        }
        print $filehandle ");\n";
        print $filehandle "\n";
    }
}

sub writePerlCodeForModule($$$@)
{
    my ($filehandle, $headerFile, $moduleName, @enums) = @_;
    
    my $time = localtime();
    my $moduleStart = << "EOT";
# This Perl module was generated: $time
# by using the 'enumsToPerl' script on the following header file:
# $headerFile

package $moduleName;
EOT
    my $moduleCode = <<'EOT';
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT = do {
    no strict 'refs';
    map { '%' . $_ } keys %{ __PACKAGE__ . '::' };
};

EOT

    print $filehandle "$moduleStart\n";
    print $filehandle "$moduleCode\n";
}

# MAIN
{
    die "Usage: enumsToPerl headerFile moduleFile\n" unless scalar(@ARGV) == 2;
    my $headerFile = $ARGV[0];
    my $moduleFile = $ARGV[1];
    die "Can't read header file '$headerFile'" unless -r $headerFile;
    my ($basename, $dirname, $ext) = fileparse($moduleFile, qr/\.[^.]*/);
    die "Can't create module file '$moduleFile'" unless -w $dirname;
    my $moduleName = $basename;
           
    undef $/;  # slurp mode
    open(HEADERFILE, "< $headerFile")
           or die "Can't open header file '$headerFile' : $!\n";
    my $contents = <HEADERFILE>;
    close(HEADERFILE) or die "Failed to close header file: $!\n";
    
    open(MODULEFILE, "> $moduleFile")
           or die "Can't open module file '$moduleFile' for writing: $!\n";
    my @enums = getEnumsFromString($contents);
    my $useHex = 0;
    writePerlCodeForModule(*MODULEFILE, $headerFile, $moduleName, @enums);
    writePerlCodeForEnums(*MODULEFILE, $useHex, @enums);
    writePerlCodeForReverseEnums(*MODULEFILE, $useHex, @enums);
    close(MODULEFILE) or die "Failed to close module file: $!\n";
}

