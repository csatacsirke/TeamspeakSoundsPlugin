<?php

//$filename = $_GET["filename"];
$channel = $_GET["channel"];


if(!isset($channel)) {
    $channel = "stable";
}

if($channel === "beta") {
    $filename = "../releases/beta/SoundplayerPlugin_x64.ts3_plugin";
} else if ($channel === "old_stable") {
    $filename = "../releases/old/SoundplayerPlugin_x64.ts3_plugin";
} else {
    $filename = "../releases/stable/SoundplayerPlugin_x64.ts3_plugin";
}


// $zip = ZipArchive::open( $filename );

$zip = new ZipArchive;
$res = $zip->open($filename);
if ($res === TRUE) {
    $package_ini_contents = $zip->getFromName('package.ini');

    // Version = 19.09.10
    $preg_result = preg_match("/Version = (.*)/", $package_ini_contents, &$matches);
    if($preg_result) {
        echo trim($matches[1]);
    }
    $zip->close();
} else {
    http_response_code(404);
    echo 'failed, code:' . $res;
}

?>
