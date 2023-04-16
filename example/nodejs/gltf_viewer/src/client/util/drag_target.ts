export const setupDragDrop = (elementId: string, className: string, load: (file: File, event: ProgressEvent<FileReader>) => void) => {
    const holder = document.getElementById(elementId);
    if (!holder) {
        return;
    } 
    holder.ondragover = function(){
        // @ts-ignore
        this.className = className;
        return false;
    };
    holder.ondragend = function(){
        // @ts-ignore
        this.className = '';
        return false;
    };
    holder.ondrop = function(e){
        // @ts-ignore
        this.className = '';
        e.preventDefault();
        // @ts-ignore
        const file = e.dataTransfer.files[0];
        const reader = new FileReader();
        reader.onload = (event) => load(file, event);
        reader.readAsDataURL(file);
    }
};