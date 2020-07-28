<?php

function phylotree_loader_cmd_drush_command()
{
    $commands = [];

    $commands['phylotree'] = [
        'description' => 'Load multiple phylotrees.',
        'arguments' => [
            'dir' => 'Directory path to where the newick files live.',
            'analysis_id' => 'Analysis ID',
        ],
        'examples' => [
            'drush phylotree /files/files/sequences/trees/my_trees 1432' => 'Where my_trees is a directory containing all the phylotree newick files and 1432 is the analysis id.',
        ],
    ];

    return $commands;
}

function phylotree_make_download_table($name, $path)
{
    return <<<html
<div><strong>Download Multiple Sequence Alignments</strong></div>
<table class="table table-valign-middle table-sm table-hover">
	<tbody>                      
		<tr class="no-border">
		<td class="text-muted download-td"><i class="fa fa-file-text-o"></i></td>
		<th>
			<a href="$path" target="_blank" download="$name.fa">$name</a>
			<div>
				<a class="text-sm text-muted font-normal" href="$path" target="_blank" download="$name.fa">Multiple sequence alignments</a>
			</div>
		</th>
		<td class="text-right"><a href="$path" class="btn btn-primary"><i class="fa fa-download"></i></a></td>
		</tr>
	</tbody>
</table>
html;
}

function drush_phylotree_loader_cmd_phylotree($dir, $analysis_id)
{
    $analysis = chado_query("select * from {analysis} where analysis_id=:id",
        [':id' => $analysis_id])->fetchObject();
    if (! $analysis) {
        drupal_set_message('Unable to find analysis with ID '.$analysis_id, 'error');

        return 1;
    }

    $trees = glob($dir.'/*.txt');

    echo "Processing: ".count($trees)." files\n";

    foreach ($trees as $tree) {
        $name = explode('/', $tree);
        $name = $name[count($name) - 1];
        $name = str_replace('.txt', '', str_replace('_tree.txt', '', $name));

        $start = strpos($tree, '/sites');
        $path = substr($tree, $start);
        $parts = explode('/', $path);
        array_pop($parts);
        $msa = implode('/', $parts).'/MultipleSequenceAlignments/'.$name.'.fa';

        echo "Creating $name\n";

        $options = [
            'name' => $name.' '.date('M jS, Y'),
            'description' => phylotree_make_download_table($name, $msa),
            'analysis_id' => $analysis_id,
            'leaf_type' => 'mRNA',
            'tree_file' => $tree,
            'format' => 'newick',
            'dbxref' => '',
            'match' => '',
            'name_re' => '',
            'load_now' => true,
        ];

        $errors = [];
        $warnings = [];

        chado_insert_phylotree($options, $errors, $warnings);

        foreach ($warnings as $warning) {
            drush_print('WARNING: '.$warning);
            drupal_set_message($warning, 'warning');
        }

        foreach ($errors as $error) {
            drush_print('ERROR: '.$error);

            drupal_set_message($error, 'error');
        }
    }

    drupal_set_message('Loading completed');
}