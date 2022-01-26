#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ log-dlproc-life-973.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2022 Tomi Ollila
#	    All rights reserved
#
# Created: Sat 01 Jan 2022 22:25:10 EET too
# Last modified: Wed 26 Jan 2022 19:23:25 +0200 too

# SPDX-License-Identifier: BSD-2-Clause

use 5.8.1;
use strict;
use warnings;

use Cwd qw/abs_path/;
use POSIX qw/dup2/;

die "\nUsage: $0 LOGFILE_PFX '.' [NAME=VALUE]... [COMMAND [ARG]...]\n\n"
  unless @ARGV > 2;

die "\nSeparator arg #2 '.' (between '$ARGV[0]' and '$ARGV[2]')",
  " missing from command line\n\n" unless $ARGV[1] eq '.';

my $logfile = shift; shift;
$ENV{$1} = $2, shift while (@ARGV and $ARGV[0] =~ /(.+)=(.+)/);

die "\ncommand [arg]... not given\n\n" unless @ARGV;

my $ld_preload = (-l $0)? readlink $0: $0;  $ld_preload =~ s|/[^/]+$||;
$ld_preload =~ s|/more$|/|; # XXX due to how this is currently laid in repo XXX
$ld_preload = '..' if $ld_preload eq $0 or $ld_preload eq '.'; # XXX ditto XXX
#$ld_preload = '.' if $ld_preload eq $0; # this instead of above if in same dir
$ld_preload = abs_path $ld_preload . '/ldpreload-log-dlproc-life-973.so';

die "\n'$ld_preload' does not exist\n",
  "\nExecute  sh ldpreload-log-dlproc-life.c 973  to create it\n\n"
  unless -f $ld_preload;

unless ($logfile =~ /[.]js0n$/) {
    $logfile .= ($logfile =~ /[.]$/) ? 'json' : '.json';
}

open O, '>', $logfile or die "\nCannot open '$logfile': $!\n\n";
POSIX::dup2(fileno(O), 973);
# close-on-exec will close fileno(O)

my $env_ld_preload = $ENV{LD_PRELOAD} || '';
if ($env_ld_preload) {
    $ENV{LD_PRELOAD} = $ld_preload . ':' . $env_ld_preload;
} else {
    $ENV{LD_PRELOAD} = $ld_preload;
}

exec @ARGV;
