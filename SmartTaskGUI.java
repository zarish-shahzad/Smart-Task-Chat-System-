import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;

/*
============================================================
 SMARTTASK FULL JAVA GUI (C++ FEATURE PARITY VERSION)
============================================================
Features Included:
- Login/Register system
- Task CRUD (Add/Delete/Update)
- Assign tasks
- Priority + Deadline
- Overdue detection
- Search
- Chat (file-based + viewer)
- Live stats dashboard
- HTML report generator
- File persistence
============================================================
*/

// ===================== TASK =====================
class Task {
    int id;
    String desc, owner, assignee, deadline, priority, status, createdAt;

    Task(int id, String desc, String owner) {
        this.id = id;
        this.desc = desc;
        this.owner = owner;
        this.assignee = owner;
        this.deadline = "none";
        this.priority = "medium";
        this.status = "pending";
        this.createdAt = new SimpleDateFormat("yyyy-MM-dd").format(new Date());
    }

    Task(int id, String desc, String owner, String assignee,
         String deadline, String priority, String status, String createdAt) {
        this.id = id;
        this.desc = desc;
        this.owner = owner;
        this.assignee = assignee;
        this.deadline = deadline;
        this.priority = priority;
        this.status = status;
        this.createdAt = createdAt;
    }

    boolean isOverdue() {
        if (deadline.equals("none") || status.equals("done")) return false;
        return deadline.compareTo(new SimpleDateFormat("yyyy-MM-dd").format(new Date())) < 0;
    }

    String serialize() {
        return id+"|"+desc+"|"+owner+"|"+assignee+"|"+deadline+"|"+priority+"|"+status+"|"+createdAt;
    }
}

// ===================== FILE =====================
class FileManager {
    String taskFile = "smarttaskdata.txt";
    String userFile = "smarttask_users.txt";
    String chatFile = "smarttask_chat.txt";

    void saveTasks(java.util.List<Task> tasks) {
        try {
            PrintWriter pw = new PrintWriter(taskFile);
            for (Task t : tasks) pw.println(t.serialize());
            pw.close();
        } catch (Exception e) { e.printStackTrace(); }
    }

    java.util.List<Task> loadTasks() {
        java.util.List<Task> list = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new FileReader(taskFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                try {
                    String[] p = line.split("\\|");
                    if (p.length < 8) continue;
                    list.add(new Task(
                            Integer.parseInt(p[0]), p[1], p[2], p[3],
                            p[4], p[5], p[6], p[7]
                    ));
                } catch (Exception ignored) {}
            }
        } catch (Exception ignored) {}
        return list;
    }

    Map<String,String> loadUsers() {
        Map<String,String> map = new HashMap<>();
        try (BufferedReader br = new BufferedReader(new FileReader(userFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] p = line.split("\\|");
                if (p.length >= 2) map.put(p[0], p[1]);
            }
        } catch (Exception ignored) {}
        return map;
    }

    void saveUser(String u, String p) {
        try {
            FileWriter fw = new FileWriter(userFile, true);
            fw.write(u+"|"+p+"\n");
            fw.close();
        } catch (Exception e) {}
    }
}

// ===================== CHAT =====================
class ChatManager {
    File file = new File("smarttask_chat.txt");

    void send(String from, String to, String msg) {
        try {
            FileWriter fw = new FileWriter(file, true);
            fw.write("["+new Date()+"] "+from+" -> "+to+": "+msg+"\n");
            fw.close();
        } catch (Exception e) {}
    }

    String read() {
        try {
            if (!file.exists()) return "No chat";
            return new String(java.nio.file.Files.readAllBytes(file.toPath()));
        } catch (Exception e) {
            return "Error";
        }
    }
}

// ===================== MAIN GUI =====================
public class SmartTaskGUI extends JFrame {

    FileManager fm = new FileManager();
    ChatManager cm = new ChatManager();

    java.util.List<Task> tasks;
    Map<String,String> users;

    String currentUser;
    int idCounter = 1;

    DefaultTableModel model;
    JTable table;

    public SmartTaskGUI() {
        users = fm.loadUsers();
        tasks = fm.loadTasks();

        for (Task t : tasks)
            idCounter = Math.max(idCounter, t.id+1);

        loginScreen();
    }

    // ================= LOGIN =================
    void loginScreen() {
        JTextField u = new JTextField();
        JPasswordField p = new JPasswordField();

        Object[] msg = {
                "Username:", u,
                "Password:", p
        };

        int opt = JOptionPane.showConfirmDialog(null,msg,"Login/Register",JOptionPane.OK_CANCEL_OPTION);
        if (opt != JOptionPane.OK_OPTION) System.exit(0);

        String user = u.getText();
        String pass = new String(p.getPassword());

        if (!users.containsKey(user)) {
            users.put(user, pass);
            fm.saveUser(user, pass);
        } else if (!users.get(user).equals(pass)) {
            JOptionPane.showMessageDialog(null,"Wrong Password");
            System.exit(0);
        }

        currentUser = user;
        initGUI();
    }

    // ================= GUI =================
    void initGUI() {
        setTitle("SmartTask FULL SYSTEM @"+currentUser);
        setSize(1000,600);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        model = new DefaultTableModel(new String[]{"ID","Desc","Owner","Assignee","Priority","Deadline","Status"},0);
        table = new JTable(model);
        refresh();

        add(new JScrollPane(table),BorderLayout.CENTER);

        JPanel panel = new JPanel();

        JButton add = new JButton("Add");
        JButton del = new JButton("Delete");
        JButton done = new JButton("Done");
        JButton assign = new JButton("Assign");
        JButton search = new JButton("Search");
        JButton chat = new JButton("Chat");
        JButton stats = new JButton("Stats");
        JButton report = new JButton("Report");

        panel.add(add);
        panel.add(del);
        panel.add(done);
        panel.add(assign);
        panel.add(search);
        panel.add(chat);
        panel.add(stats);
        panel.add(report);

        add(panel,BorderLayout.SOUTH);

        add.addActionListener(e->addTask());
        del.addActionListener(e->delete());
        done.addActionListener(e->markDone());
        assign.addActionListener(e->assign());
        search.addActionListener(e->search());
        chat.addActionListener(e->chat());
        stats.addActionListener(e->stats());
        report.addActionListener(e->report());

        setVisible(true);
    }

    // ================= TASK OPS =================
    void addTask(){
        String d=JOptionPane.showInputDialog("Task");
        if(d==null)return;
        tasks.add(new Task(idCounter++,d,currentUser));
        save();refresh();
    }

    void delete(){
        int r=table.getSelectedRow();if(r==-1)return;
        int id=(int)model.getValueAt(r,0);
        tasks.removeIf(t->t.id==id);
        save();refresh();
    }

    void markDone(){
        int r=table.getSelectedRow();if(r==-1)return;
        int id=(int)model.getValueAt(r,0);
        for(Task t:tasks) if(t.id==id)t.status="done";
        save();refresh();
    }

    void assign(){
        int r=table.getSelectedRow();if(r==-1)return;
        String u=JOptionPane.showInputDialog("Assign to");
        int id=(int)model.getValueAt(r,0);
        for(Task t:tasks) if(t.id==id)t.assignee=u;
        save();refresh();
    }

    void search(){
        String k=JOptionPane.showInputDialog("Keyword");
        model.setRowCount(0);
        for(Task t:tasks)
            if(t.desc.toLowerCase().contains(k.toLowerCase()))
                model.addRow(row(t));
    }

    void chat(){
        String to=JOptionPane.showInputDialog("To");
        String m=JOptionPane.showInputDialog("Msg");
        cm.send(currentUser,to,m);
        JOptionPane.showMessageDialog(this,cm.read());
    }

    void stats(){
        int done=0,over=0;
        for(Task t:tasks){if(t.status.equals("done"))done++;if(t.isOverdue())over++;}
        JOptionPane.showMessageDialog(this,
                "Total: "+tasks.size()+"\nDone: "+done+"\nOverdue: "+over);
    }

    void report(){
        try{
            PrintWriter pw=new PrintWriter("report.html");
            pw.println("<h1>SmartTask</h1><table border=1>");
            for(Task t:tasks)
                pw.println("<tr><td>"+t.id+"</td><td>"+t.desc+"</td><td>"+t.status+"</td></tr>");
            pw.println("</table>");pw.close();
        }catch(Exception e){}
    }

    // ================= HELPERS =================
    void refresh(){
        model.setRowCount(0);
        for(Task t:tasks) model.addRow(row(t));
    }

    Object[] row(Task t){
        return new Object[]{t.id,t.desc,t.owner,t.assignee,t.priority,t.deadline,t.status};
    }

    void save(){fm.saveTasks(tasks);}

    public static void main(String[] args){
        new SmartTaskGUI();
    }
}
