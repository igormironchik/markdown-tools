
function Component()
{
}

Component.prototype.createOperations = function()
{
	component.createOperations();

	if( installer.value( "os" ) === "win" )
	{
		component.addOperation( "Execute", "{0,1638}", "@TargetDir@\\bin\\VC_redist.x64.exe",
			"/silent" );

		component.addOperation( "CreateShortcut", "@TargetDir@\\bin\\md-editor.exe",
			"@StartMenuDir@\\Markdown Editor.lnk", "workingDirectory=@TargetDir@\\bin" );
			
		component.addOperation( "CreateShortcut", "@TargetDir@\\bin\\md-pdf-gui.exe",
			"@StartMenuDir@\\Markdown To PDF Converter.lnk", "workingDirectory=@TargetDir@\\bin" );

	}
}
