<?php

use Pun\IdRex8;
use Pun\Pun8;
use Pun\Re8map;
use Pun\Recap8;

function show($result) {
	echo "Show class is " . get_class($result) . PHP_EOL;
	if ($result->count() > 1) {
		echo $result->getCap(0) . PHP_EOL;
		echo $result->getCap(1) . PHP_EOL;
	}
	else {
		echo $result->count() . " captures returned" . PHP_EOL;
	}
}
function routine() {
	$input = "=Test = value";

	$pun = new Pun8($input);

	echo "NextChar is: " . $pun->nextChar() . PHP_EOL;

	$expEquals = "^(\\h*=\\h*)";
	$expKey = "(^[-A-Z_a-z0-9]+)";

	$reg = new IdRex8(1,$expEquals);

	if (!$reg->isCompiled()) {
		echo $reg->getMessage() . PHP_EOL;
	}
	else {
		echo "Compiled OK!" . PHP_EOL;
	}
	show($reg->match($input));
	$map = new Re8map();

	$map->setIdRex(2,$expKey);
	$map->setIdRex(1,$expEquals);
	$pun->setRe8map($map);

	$matches = $pun->matchIdRex(2);
	echo "Match class is " . get_class($matches) . PHP_EOL;

	if ($matches->count() > 1) {
		echo "Captured " . $matches->getCap(1) . PHP_EOL;
		$pun->addOffset(strlen($matches->getCap(0)));
	}
	show($pun->matchIdRex(1));

	$p8 = $pun->getIdRex(1);
	echo "object is " . get_class($p8) . PHP_EOL;

	show($p8->match($input));
}

$memInc = 0.0;
$i = 0;
$startMem = $endMem = 0;
$memInit = memory_get_usage();

for ($i = 0 ; $i < 2; $i++)
{
    if ($i == 1) {
        $startMem = memory_get_usage();
    }

	routine();
}
gc_collect_cycles();
$endMem = memory_get_usage();
$memInc +=  ($endMem - $startMem);
echo "*** Memory Inc  = " . $memInc . PHP_EOL;