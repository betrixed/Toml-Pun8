<?php

use Pun\IdRex8;
use Pun\Pun8;
use Pun\Re8map;

function show($result) {

	if (!empty($result)) {
		foreach($result as $idx => $item) {
			echo $idx . ": " . $item . " length " . strlen($item) . PHP_EOL;
		}
	}
	else {
		echo "Empty result" . PHP_EOL;
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
	if (!empty($matches)) {
		echo "Matched " . $matches[0] . PHP_EOL;
		$pun->addOffset(strlen($matches[0]));
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