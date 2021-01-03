<?php 

$filename = "downloads/SoundplayerPlugin_x64_beta.ts3_plugin";

$zip = new ZipArchive;
$res = $zip->open($filename);
if ($res === TRUE) {
    $package_ini_contents = $zip->getFromName('changelog.json');
    if($package_ini_contents) {
        echo $package_ini_contents;
    } else {
        http_response_code(500);
        echo 'zipping error - ' . $res;
    }
    
    $zip->close();
} else {
    http_response_code(500);
    echo "elavult valami";
    //$redirect_url = "https://raw.githubusercontent.com/csatacsirke/TeamspeakSoundsPlugin/master/changelog.json";
    //header("Location: $redirect_url");
}



?>
