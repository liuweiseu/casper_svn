/* #pragma ident "@(#)PcdRequest.java	1.2 12/24/01 EDT" */

import java.awt.event.*;
import java.awt.*;
import java.applet.*;

public class PcdRequest extends Applet implements ActionListener
{
    static Dimension dim = null ;
    static Label spacer = null ;
    static int buttonCount = 0 ;
    static CheckboxGroup cbg = null ;
    static String curPciName = null ;
    static Frame f = null ;

    public native String PcdGetConfigInfo(String PcdConfigFile) ;
    public native String PcdGetIntfcStrings(String PciString) ;
    public native String PcdGetBoardInfoStrings() ;
    public native void PcdSetInterfaceBitfile(String BitfileString) ;
    public native int PcdSaveConfigScript() ;

    public void	init() {}

    public void	start()
    {
	System.loadLibrary("PcdRequest");

	final String infoStr = PcdGetConfigInfo("pcd_bitfiles") ;
	if (infoStr == null)
	    System.exit(1) ;

	final TextArea tx = new TextArea(infoStr, 40, 80, TextArea.SCROLLBARS_BOTH) ;
	tx.setFont(new Font("Monospaced", Font.PLAIN, 12));
	tx.setBackground(Color.lightGray) ;
	tx.setForeground(Color.black) ;
	final Frame infoFrame = new Frame("PCI and Interface Bitfiles");

	infoFrame.add("Center", tx) ;
	infoFrame.setSize(840, 500);
	infoFrame.show() ;
        infoFrame.addWindowListener(new WindowAdapter() {
	    public void	windowClosing(WindowEvent e) {
		infoFrame.setVisible(false) ;
	    }
        });

	f = new Frame("PcdRequest");

	f.setBackground(Color.lightGray) ;
	f.setForeground(Color.black) ;

	f.add("Center",	this);

        f.addWindowListener(new WindowAdapter() {
	    public void	windowClosing(WindowEvent e) {
		System.exit(0) ;
	    }
        });

	setLayout(new GridLayout(0, 1, 0, 0)) ;

	String pciString = null ;

    	if ((pciString = PcdGetBoardInfoStrings()) != null)
	    interfaceBitfileMenu(f, pciString) ;
	else
	{
	    Label spacer = new Label("") ;
            spacer.setFont(new Font("Monospaced", Font.BOLD, 12));
	    add(spacer) ;
	    Label label = new Label("Notice:  No PCD boards installed or driver not loaded") ;
            label.setFont(new Font("Monospaced", Font.BOLD, 12));
	    add(label) ;
	    label = new Label("") ;
            label.setFont(new Font("Monospaced", Font.BOLD, 12));
	    add(label) ;
	    final Button q = new Button(" Quit ") ;
	    q.setFont(new Font("Monospaced", Font.BOLD, 12));
	    q.addActionListener(this) ;
	    Panel p = new Panel() ;
	    p.add(q) ;
	    add(p) ;

	    f.pack();
	    f.show();

	    //dim = spacer.getSize() ;
	    //dim.width += 8 ;
	    //dim.height *= 6 ;
	    //f.setSize(dim) ;
	}

        Toolkit tk = Toolkit.getDefaultToolkit() ;

        // initialize screen variables
        Dimension d = tk.getScreenSize();
        Dimension mySize = f.getSize();
        int screen_x = (d.width  - mySize.width)/2 ;
        int screen_y = (d.height - mySize.height)/2 ;
	if (screen_x < 0) screen_x = 0 ;
	if (screen_y < 0) screen_y = 0 ;
        f.setLocation(screen_x, screen_y) ;
	f.show();

    }

    public void	stop()
    {
	System.out.println("Stop!") ;
	System.exit(0) ;
    }

    public static void main(String args[])
    {
	final PcdRequest first = new PcdRequest();
	first.init();
	first.start();
    }

    void interfaceBitfileMenu(Frame f, String pciInfoString)
    {
	removeAll() ;

	final Label label = new Label("             " + pciInfoString + "             ") ;
        label.setFont(new Font("Monospaced", Font.BOLD, 12));
	add(label) ;

	spacer = new Label("") ;
        spacer.setFont(new Font("Monospaced", Font.BOLD, 12));
	spacer.setEnabled(false) ;
	add(spacer) ;

	cbg = new CheckboxGroup() ;
	String intfcStr ;
	
	final Checkbox a = new Checkbox("Do Not Configure:     Applications will load the interface bitfile.          ",
													cbg, true);
	a.setFont(new Font("Monospaced", Font.BOLD, 12));
	add(a) ;
	
	curPciName = pciInfoString.substring(pciInfoString.lastIndexOf(" ") + 1) ;


	while ((intfcStr = PcdGetIntfcStrings(curPciName)) != null)
	{
	    final Checkbox b = new Checkbox(intfcStr, cbg, false) ;
	    b.setFont(new Font("Monospaced", Font.BOLD, 12));
	    //b.setActionCommand(getBitfileName(intfcStr)) ;
	    //System.out.println(b.getActionCommand()) ;
	    add(b) ;

	    ++ buttonCount ;
	}


	Label space1 = new Label("") ;
        space1.setFont(new Font("Monospaced", Font.BOLD, 12));
	add(space1) ;


	final Button next = new Button("Next >>") ;
	next.setFont(new Font("Monospaced", Font.BOLD, 14));
        next.addActionListener(this) ;
	final Button c = new Button("Cancel") ;
	c.setFont(new Font("Monospaced", Font.BOLD, 14));
        c.addActionListener(this) ;
	Panel p = new Panel() ;
	p.add(c) ;
	p.add(next) ;
	add(p) ;

	f.pack() ;
	dim = spacer.getSize() ;
	dim.height *= (buttonCount + 6) ;
	f.setSize(dim) ;

	buttonCount = 0 ;
    }
    
    String getBitfileName(String label)
    {
    	return label.substring(0, label.indexOf(':')) ;
    }

    int getChar()
    {
    	int ret = 0 ;
	try { ret = System.in.read() ; } catch (java.io.IOException e)
        {
	    System.out.println("IO Exception");
	    System.exit(0) ;
        }

	return ret ;
    }

    //
    // Take care of top level GUI button presses.
    //
    public void actionPerformed(ActionEvent e)
    {
        String item = e.getActionCommand() ;

        if (item == "Next >>")
	{
	    String bitName = getBitfileName(cbg.getSelectedCheckbox().getLabel()) ;

	    if ( ! bitName.equals("Do Not Configure"))
	        PcdSetInterfaceBitfile(bitName) ;

	    String pciString = null ;

    	    if ((pciString = PcdGetBoardInfoStrings()) != null)
	        interfaceBitfileMenu(f, pciString) ;
	    else
	    {
		removeAll() ;

	        Label label = new Label("                         PCD Request") ;
                label.setFont(new Font("Monospaced", Font.BOLD, 12));
	        add(label) ;
	        Label spacer = new Label("") ;
                spacer.setFont(new Font("Monospaced", Font.BOLD, 12));
	        add(spacer) ;
	        label = new Label("    Press Finish to save the PCD configuration script    ") ;
                label.setFont(new Font("Monospaced", Font.BOLD, 12));
	        add(label) ;
	        label = new Label("") ;
                label.setFont(new Font("Monospaced", Font.BOLD, 12));
	        add(label) ;

	        final Button c = new Button("Cancel") ;
	        c.setFont(new Font("Monospaced", Font.BOLD, 12));
	        c.addActionListener(this) ;
	        final Button fin = new Button("Finish") ;
	        fin.setFont(new Font("Monospaced", Font.BOLD, 12));
	        fin.addActionListener(this) ;

		Panel p = new Panel() ;
	        p.add(c) ;
	        p.add(fin) ;
		add(p) ;

	        f.pack();

	        dim = spacer.getSize() ;
	        dim.width += 8 ;
	        dim.height *= 6 ;
	        f.setSize(dim) ;
	    }
	}
        else if (item == "Finish")
	{
	    if (PcdSaveConfigScript() != 0)
	        ;
	    Quit() ;
	}
        else if (item == "Cancel")
	{
	    Quit() ;
	}
        else if (item == " Quit ")
	{
	    Quit() ;
	}
    }

    void Quit()
    {
	System.exit(0) ;
    }
}
