<?php

function awk($file_full_path,$delimiter)
{
    $fd = fopen($file_full_path,'r');
    if(!$fd)
    {
        echo("unable to open file!");
	return false;
    }

    $data_table = array();

    while(true)
    {
        $raw_string = fgets($fd);
	if(!$raw_string)
	{
	    break;
	}

	unset($data_row);

	$split_array = explode($delimiter,$raw_string);
	foreach($split_array as $cell)
	{
	    $cell = trim($cell);
	    if(strlen($cell) <= 0)
	    {
	        continue;
	    }

	    $data_row[] = $cell;
	}

	if(isset($data_row))
	{
	    $data_table[] = $data_row;
	}
    }

    fclose($fd);
    return $data_table;
}

?>
